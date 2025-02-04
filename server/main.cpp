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

void processPing(const Host& host, json& results) {

    const std::string command = "ping -c 1 -W 1 " + host.ip + " 2>&1";

    FILE* pipe = popen(command.c_str(), "r");
    if (!pipe) {

        std::cerr << "Failed to run ping command for " << host.name << std::endl;

        return;

    }

    char buffer[256];
    std::string result;
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        result += buffer;
    }

    pclose(pipe);

    double latency = -1.0;
    const std::regex latencyRegex("time=([0-9]+\\.?[0-9]*) ms");
    if (std::smatch match; std::regex_search(result, match, latencyRegex)) {
        latency = std::stod(match[1]);
    }

    std::lock_guard lock(results_mutex);

    results.push_back({
        {"name", host.name},
        {"ip", host.ip},
        {"latency_ms", latency >= 0 ? latency : -1}
    });

}

void monitorHost(const Host& host, json& results) {

    const int sock = socket(AF_INET, SOCK_STREAM, 0);

    if (sock < 0) {

        std::cerr << "Error: Could not create socket for " << host.name << std::endl;

        return;

    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(6799);

    if (inet_pton(AF_INET, host.ip.c_str(), &serverAddr.sin_addr) <= 0) {

        std::cerr << "Invalid IP address for " << host.name << std::endl;

        close(sock);

        return;

    }

    if (connect(sock, reinterpret_cast<sockaddr *>(&serverAddr), sizeof(serverAddr)) < 0) {

        std::lock_guard lock(results_mutex);

        results.push_back({
            {"name", host.name},
            {"ip", host.ip},
            {"status", "connection failed"}
        });

        close(sock);

        return;

    }

    char buffer[1024] = {};
    if (const int bytesRead = read(sock, buffer, sizeof(buffer) - 1); bytesRead > 0) {

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

    close(sock);

}
void saveResults(const std::string& filename, const json& data) {

    std::lock_guard lock(results_mutex);

    std::ofstream file(filename, std::ios::trunc);
    if (!file) {

        std::cerr << "Error: Could not write to " << filename << std::endl;

        return;

    }

    file << data.dump(4);

    file.close();

}

void runCycle(const std::string& inputFile, const std::string& outputFile, const int sleepIntervalSeconds, void (*task)(const Host&, json&)) {

    while (keepRunning) {

        std::vector<Host> hosts = loadHosts(inputFile);
        json results = json::array();

        std::vector<std::thread> threads;
        for (const auto& host : hosts) {
            threads.emplace_back(task, host, std::ref(results));
        }

        for (auto& t : threads) {
            t.join();
        }

        saveResults(outputFile, results);

        std::this_thread::sleep_for(std::chrono::seconds(sleepIntervalSeconds));

    }

}

int main() {

    const std::string inputFile = "/etc/gteam/dynamic/supervisor/config/hosts.json";

    std::string pingOutputFile = "/etc/gteam/dynamic/supervisor/results/ping_results.json";
    std::string monitorOutputFile = "/etc/gteam/dynamic/supervisor/results/monitor_results.json";

    if (loadHosts(inputFile).empty()) {

        std::cerr << "No hosts found in JSON file.\n";

        return 1;

    }

    int sleepIntervalSeconds = 2;
    std::thread pingCycleThread(runCycle, inputFile, pingOutputFile, sleepIntervalSeconds, processPing);
    std::thread monitorCycleThread(runCycle, inputFile, monitorOutputFile, sleepIntervalSeconds, monitorHost);

    pingCycleThread.join();
    monitorCycleThread.join();

    return 0;

}
