#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <nlohmann/json.hpp>
#include <regex>
#include <cstdio>

using json = nlohmann::json;

struct Host {
    std::string name;
    std::string ip;
};

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

    const std::string command = "ping -c 1 " + host.ip + " 2>&1";

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

    return -1.0; // Failure

}

void saveResults(const std::vector<json>& results, const std::string& filename) {

    std::ofstream file(filename);
    if (!file) {

        std::cerr << "Error: Could not write to " << filename << std::endl;

        return;

    }

    file << json(results).dump(4); // Pretty print JSON
    file.close();

}

int main() {

    const std::string inputFile = "./config/hosts.json";
    const std::string outputFile = "./results/ping_results.json";

    while (true) {
        std::vector<Host> hosts = loadHosts(inputFile);
        if (hosts.empty()) {

            std::cerr << "No hosts found in JSON file.\n";

            return 1;

        }

        std::vector<json> results;

        for (const auto& host : hosts) {

            double latency = pingHost(host);

            json entry = {
                {"name", host.name},
                {"ip", host.ip},
                {"latency_ms", latency >= 0 ? json(latency) : json(nullptr)}  // Correct way to assign null
            };

            results.push_back(entry);

            saveResults(results, outputFile);

        }

    }

}
