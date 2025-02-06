#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <thread>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <nlohmann/json.hpp>
#include <atomic>
#include <mutex>
#include <chrono>

#define PORT 6799
#define BUFFER_SIZE 4096

using json = nlohmann::json;

std::atomic keepRunning(true);
std::mutex file_mutex;

std::string readFileContent(const std::string& filePath) {

    std::ifstream file(filePath);
    if (!file) {

        std::cerr << "Error: Could not open " << filePath << std::endl;

        return "";

    }

    return std::string((std::istreambuf_iterator(file)), std::istreambuf_iterator<char>());

}

std::string getAllowedIP(const std::string& configPath) {

    std::ifstream file(configPath);
    if (!file) {

        std::cerr << "Error: Could not open " << configPath << std::endl;

        return "";

    }

    json configJson;
    file >> configJson;
    return configJson["allowed_ip"].get<std::string>();

}

void handleClient(const int clientSocket, const std::string& allowedIP) {

    sockaddr_in clientAddr{};
    socklen_t addrLen = sizeof(clientAddr);
    getpeername(clientSocket, reinterpret_cast<sockaddr*>(&clientAddr), &addrLen);

    const std::string clientIP = inet_ntoa(clientAddr.sin_addr);
    std::cout << "Connection from: " << clientIP << std::endl;

    if (clientIP != allowedIP) {

        std::cerr << "Unauthorized access from " << clientIP << std::endl;

        const auto rejectMessage = "403 Forbidden: Unauthorized access\n";

        send(clientSocket, rejectMessage, strlen(rejectMessage), 0);

        close(clientSocket);

        return;

    }

    const std::string statsContent = readFileContent("/etc/gteam/dynamic/supervisor/client/results/monitor_results.json");
    if (statsContent.empty()) {

        const auto errorMessage = "500 Internal Server Error: Could not read stats file\n";

        send(clientSocket, errorMessage, strlen(errorMessage), 0);

    } else {

        send(clientSocket, statsContent.c_str(), statsContent.length(), 0);

    }

    close(clientSocket);

}

void runServer(const std::string& configPath) {

    const int serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    if (serverSocket == -1) {

        std::cerr << "Failed to create socket" << std::endl;

        return;

    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(PORT);

    if (bind(serverSocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) < 0) {

        std::cerr << "Failed to bind socket" << std::endl;

        close(serverSocket);

        return;

    }

    if (listen(serverSocket, 5) < 0) {

        std::cerr << "Failed to listen on socket" << std::endl;

        close(serverSocket);

        return;

    }

    std::cout << "Server listening on port " << PORT << std::endl;

    std::string allowedIP = getAllowedIP(configPath);
    if (allowedIP.empty()) {

        std::cerr << "Failed to retrieve allowed IP from hosts.json" << std::endl;

        return;

    }

    while (keepRunning) {

        sockaddr_in clientAddr{};
        socklen_t addrLen = sizeof(clientAddr);
        int clientSocket = accept(serverSocket, reinterpret_cast<struct sockaddr*>(&clientAddr), &addrLen);

        if (clientSocket < 0) {

            std::cerr << "Failed to accept connection" << std::endl;

            continue;

        }


        std::thread(handleClient, clientSocket, allowedIP).detach();

    }

    close(serverSocket);

}

double getCpuUsage() {

    static long previousIdleTime = 0;
    static long previousTotalTime = 0;

    std::ifstream file("/proc/stat");
    if (!file) {

        std::cerr << "Error: Could not open /proc/stat" << std::endl;

        return -1.0;

    }

    std::string line;
    std::getline(file, line);
    std::istringstream ss(line);

    std::string cpuLabel;
    long user, nice, system, idle, iowait, irq, softirq, steal;
    ss >> cpuLabel >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> steal;

    long idleTime = idle + iowait;
    long totalTime = user + nice + system + idle + iowait + irq + softirq + steal;

    long deltaIdle = idleTime - previousIdleTime;
    long deltaTotal = totalTime - previousTotalTime;

    previousIdleTime = idleTime;
    previousTotalTime = totalTime;

    if (deltaTotal == 0) return 0.0;
    return (1.0 - static_cast<double>(deltaIdle) / deltaTotal) * 100.0;

}

void monitorCpuUsage(const std::string& resultsFile) {

    while (keepRunning) {

        double cpuUsage = getCpuUsage();

        json cpuStats = {
            {"timestamp", std::chrono::system_clock::to_time_t(std::chrono::system_clock::now())},
            {"cpu_usage_percent", cpuUsage}
        };

        std::lock_guard lock(file_mutex);
        if (std::ofstream file(resultsFile, std::ios::trunc); file) {

            file << cpuStats.dump(4);

        } else {

            std::cerr << "Error: Could not write to " << resultsFile << std::endl;
        }


        std::this_thread::sleep_for(std::chrono::seconds(1));

    }

}

int main() {

    const std::string configPath = "/etc/gteam/dynamic/supervisor/client/config/hosts.json";
    const std::string resultsFile = "/etc/gteam/dynamic/supervisor/client/results/monitor_results.json";

    std::thread serverThread(runServer, configPath);
    std::thread cpuMonitoringThread(monitorCpuUsage, resultsFile);

    serverThread.join();
    cpuMonitoringThread.join();

    return 0;

}
