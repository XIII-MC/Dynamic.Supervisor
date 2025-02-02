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

using json = nlohmann::json;

struct Host {
    std::string name;
    std::string ip;
};

std::mutex results_mutex;
std::vector<json> results;
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

double pingHost(const Host& host) {

    const std::string command = "ping -c 1 -W 1 " + host.ip + " 2>&1";

    FILE* pipe = popen(command.c_str(), "r");
    if (!pipe) {

        std::cerr << "Failed to run ping command for " << host.name << std::endl;

        return -1.0;

    }

    char buffer[256];
    std::string result;
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        result += buffer;
    }

    pclose(pipe);

    const std::regex latencyRegex("time=([0-9]+\\.?[0-9]*) ms");
    if (std::smatch match; std::regex_search(result, match, latencyRegex)) {
        return std::stod(match[1]);
    }

    return -1.0; // Failure (timeout)
}

void processHost(const Host& host) {

    double latency = pingHost(host);

    std::lock_guard lock(results_mutex);

    const json entry = {
        {"name", host.name},
        {"ip", host.ip},
        {"latency_ms", latency >= 0 ? latency : -1}  // Mark as -1 if host failed to respond
    };

    results.push_back(entry);

}

void saveResults(const std::string& filename) {

    std::lock_guard lock(results_mutex);

    // Open the output file and overwrite it with the latest results
    std::ofstream file(filename, std::ios::trunc);
    if (!file) {

        std::cerr << "Error: Could not write to " << filename << std::endl;

        return;

    }

    // Write the updated results
    file << json(results).dump(4);
    file.close();
}

void runPingCycle(const std::string &inputFile, const std::string& outputFile, const int sleepIntervalSeconds) {

    while (keepRunning) {

        // Load hosts to ping
        std::vector<Host> hosts = loadHosts(inputFile);

        std::vector<std::thread> threads;

        for (const auto& host : hosts) {
            threads.emplace_back(processHost, host);  // Start a thread for each host
        }

        // Wait before the next round
        std::this_thread::sleep_for(std::chrono::seconds(sleepIntervalSeconds));

        // Wait for all threads to complete
        for (auto& t : threads) {
            t.detach();
        }

        // Save results to file
        saveResults(outputFile);

        // Clear previous results (to avoid keeping old data)
        {
            std::lock_guard lock(results_mutex);
            results.clear();
        }
    }
}

int main() {

    const std::string inputFile = "../../web/hosts.json";
    std::string outputFile = "../../web/ping_results.json";

    // Load hosts from the JSON file
    if (loadHosts(inputFile).empty()) {

        std::cerr << "No hosts found in JSON file.\n";

        return 1;

    }

    int sleepIntervalSeconds = 1;

    // Start the ping cycle in a separate thread
    std::thread pingCycleThread(runPingCycle, inputFile, outputFile, sleepIntervalSeconds);

    // Keep running the ping cycle
    pingCycleThread.join();

    return 0;
}
