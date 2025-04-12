#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <nlohmann/json.hpp>
#include <regex>
#include <cstdio>
#include <thread>
#include <mutex>
#include <chrono>
#include <atomic>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <filesystem>  // Required for filesystem operations

#define PORT 6799

using json = nlohmann::json;

struct Host {
    std::string name;
    std::string ip;
};

std::mutex results_mutex;
std::atomic keepRunning(true);

std::vector<Host> loadHosts(const std::string& filename) {

    std::ifstream file(filename);

    if (!file) {

        std::cerr << "Error: Could not open " << filename << std::endl;

        return {};

    }

    json jsonData;
    file >> jsonData;

    std::vector<Host> hosts;
    for (const auto& item : jsonData) {
        hosts.push_back({item["name"], item["ip"]});
    }

    return hosts;

}

void processPing(const Host &host, json&) {

    const std::string command = "ping -c 1 -W 1 " + host.ip + " 2>&1";

    // Run command
    FILE* pipe = popen(command.c_str(), "r");
    if (!pipe) {

        std::cerr << "Failed to run ping command for " << host.name << std::endl;

        return;

    }

    // Define buffer and output
    char buffer[256];
    std::string result;
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        result += buffer;
    }

    pclose(pipe);

    // Extract latency from command
    double latency = -1.0;
    const std::regex latencyRegex("time=([0-9]+\\.?[0-9]*) ms");
    if (std::smatch match; std::regex_search(result, match, latencyRegex)) {
        latency = std::stod(match[1]);
    }

    std::filesystem::create_directory("/etc/gteam/dynamic/supervisor/server/results/" + host.name);

    // Lock and save the ping result to the file in the host's subfolder
    {

        std::string filePath = "/etc/gteam/dynamic/supervisor/server/results/" + host.name + "/ping-results.json";
        std::lock_guard lock(results_mutex);

        json pingResult = {
            {"timestamp", std::chrono::system_clock::to_time_t(std::chrono::system_clock::now())},
            {"latency", latency >= 0 ? latency : -1}
        };

        // Write results to file
        if (std::ofstream outFile(filePath, std::ios::trunc); outFile) {

            outFile << pingResult.dump(4);

        } else {

            std::cerr << "Error: Could not write to " << filePath << std::endl;

        }

    }

}

void monitorHost(const Host& host, json& results) {

    // Base vars
    sockaddr_storage serverAddr{};
    socklen_t addr_len;
    int sock;

    // Create IPv4 socket
    if (inet_pton(AF_INET, host.ip.c_str(), &(reinterpret_cast<sockaddr_in*>(&serverAddr)->sin_addr)) == 1) {

        // Create socket
        sock = socket(AF_INET, SOCK_STREAM, 0);
        addr_len = sizeof(sockaddr_in);
        reinterpret_cast<sockaddr_in*>(&serverAddr)->sin_family = AF_INET;
        reinterpret_cast<sockaddr_in*>(&serverAddr)->sin_port = htons(PORT);

    // Create IPv6 socket
    } else if (inet_pton(AF_INET6, host.ip.c_str(), &(reinterpret_cast<sockaddr_in6*>(&serverAddr)->sin6_addr)) == 1) {

        // Create socket
        sock = socket(AF_INET6, SOCK_STREAM, 0);
        addr_len = sizeof(sockaddr_in6);
        reinterpret_cast<sockaddr_in6*>(&serverAddr)->sin6_family = AF_INET6;
        reinterpret_cast<sockaddr_in6*>(&serverAddr)->sin6_port = htons(PORT);

    // If the user doesn't know how to type an IP
    } else {

        std::cerr << "Invalid IP address format for " << host.name << std::endl;

        return;

    }

    // No good no good
    if (sock < 0) {

        std::cerr << "Error: Could not create socket for " << host.name << std::endl;

        return;

    }

    // Configure socket
    timeval timeout{};
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));

    // Connect
    if (const int result = connect(sock, reinterpret_cast<sockaddr*>(&serverAddr), addr_len); result < 0 && errno != EINPROGRESS) {

        std::cerr << "Connection failed immediately for " << host.ip << std::endl;

        close(sock);

        return;

    }

    fd_set write_fds;
    FD_ZERO(&write_fds);
    FD_SET(sock, &write_fds);

    // Check if the port is open and we are allowed to connect
    if (const int select_result = select(sock + 1, nullptr, &write_fds, nullptr, &timeout); select_result <= 0) {

        std::cerr << "Connection timed out or failed for " << host.ip << std::endl;

        std::lock_guard lock(results_mutex);
        results.push_back({
            {"name", host.name},
            {"ip", host.ip},
            {"status", "connection failed"}
        });

        close(sock);

        return;

    }

    // Extract data and write it
    char buffer[1024] = {};
    if (const long bytesRead = read(sock, buffer, sizeof(buffer) - 1); bytesRead > 0) {

        std::lock_guard lock(results_mutex);
        results.push_back({
            {"name", host.name},
            {"ip", host.ip},
            {"status", "connection successful"},
            {"response", std::string(buffer, bytesRead)}
        });

    } else {

        std::lock_guard lock(results_mutex);
        results.push_back({
            {"name", host.name},
            {"ip", host.ip},
            {"status", "no response"}
        });

    }

    // Create the results folder for the host if it doesn't exist
    std::filesystem::create_directory("results");

    // Create the host-specific folder
    std::filesystem::create_directory("/results/" + host.name);

    // Define file path for storing the monitoring results

    // Lock and save the monitor result to the file in the host's subfolder
    {
        std::string filePath = "/etc/gteam/dynamic/supervisor/server/results/" + host.name + "/monitor-results.json";
        std::lock_guard lock(results_mutex);

        // Save the results to the file
        if (std::ofstream outFile(filePath, std::ios::trunc); outFile) {

            outFile << results.dump(4);

        } else {

            std::cerr << "Error: Could not write to " << filePath << std::endl;

        }
    }

    close(sock);

}

void runCycle(const std::string& inputFile, const int sleepIntervalSeconds, void (*task)(const Host&, json&)) {

    while (keepRunning) {

        // Loop through each host
        for (std::vector<Host> hosts = loadHosts(inputFile); const auto& host : hosts) {
            json results;

            // Create a lambda function that captures 'host' and 'results' and calls the task
            auto threadTask = [host, &results, task]() mutable {
                task(host, results);
            };

            // Run the task for the current host in a separate thread
            std::thread(threadTask).detach();

            // Wait x seconds before next run
            std::this_thread::sleep_for(std::chrono::seconds(sleepIntervalSeconds));
        }

    }

}

int main() {

    const std::string inputFile = "/etc/gteam/dynamic/supervisor/server/config/hosts.json";

    int sleepIntervalSeconds = 2;
    std::thread pingCycleThread(runCycle, inputFile, sleepIntervalSeconds, processPing);
    std::thread monitorCycleThread(runCycle, inputFile, sleepIntervalSeconds, monitorHost);

    pingCycleThread.join();
    monitorCycleThread.join();

    return 0;

}
