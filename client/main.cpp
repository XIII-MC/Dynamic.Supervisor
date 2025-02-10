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
#define CONFIG_PATH "/etc/gteam/dynamic/supervisor/client/config/hosts.json"
#define RESULTS_PATH "/etc/gteam/dynamic/supervisor/client/results/monitor_results.json"

using json = nlohmann::json;

std::atomic keepRunning(true);
std::mutex file_mutex;

std::string getResults() {

    std::ifstream file(RESULTS_PATH);
    if (!file) {

        std::cerr << "Error: Could not open " << RESULTS_PATH << std::endl;

        return "";

    }

    return std::string((std::istreambuf_iterator(file)), std::istreambuf_iterator<char>());

}

std::string getConfigValue(const std::string& key) {

    std::ifstream configFile(CONFIG_PATH);
    if (!configFile) {

        std::cerr << "Error: Could not open config file: " << CONFIG_PATH << std::endl;

        return "";

    }

    try {

        json configJson;
        configFile >> configJson;
        if (configJson.contains(key)) {

            return configJson[key].get<std::string>();

        }

        std::cerr << "Error: Key '" << key << "' not found in config file" << std::endl;

    } catch (const std::exception& e) {

        std::cerr << "Error: Failed to parse config file: " << e.what() << std::endl;

    }

    return "";

}

void handleClient(const int clientSocket, const std::string& allowedIP, const std::string& clientIP) {

    sockaddr_in clientAddr{};
    socklen_t addrLen = sizeof(clientAddr);
    getpeername(clientSocket, reinterpret_cast<sockaddr*>(&clientAddr), &addrLen);

    std::cout << "Client connected: " << inet_ntoa(clientAddr.sin_addr) << std::endl;

    if (clientIP != allowedIP) {

        std::cerr << "Unauthorized access from " << clientIP << std::endl;

        const auto rejectMessage = "403 Forbidden: Unauthorized access\n";

        send(clientSocket, rejectMessage, strlen(rejectMessage), 0);

        close(clientSocket);

        return;

    }

    const std::string statsContent = getResults();
    if (statsContent.empty()) {

        const auto errorMessage = "500 Internal Server Error: Could not read stats file\n";

        send(clientSocket, errorMessage, strlen(errorMessage), 0);

    } else {

        send(clientSocket, statsContent.c_str(), statsContent.length(), 0);

    }

    close(clientSocket);

}

void runServer() {

    const std::string allowedIp = getConfigValue("allowed_ip");

    int serverSocket;
    if (allowedIp.contains(".")) {

        serverSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (serverSocket < 0) {

            std::cerr << "Failed to create IPv4 socket" << std::endl;

            exit(-1);

        }

        sockaddr_in serverAddr{};
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_addr.s_addr = INADDR_ANY;
        serverAddr.sin_port = htons(PORT);

        if (bind(serverSocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) < 0) {

            std::cerr << "Failed to bind IPv4 socket on port " << PORT << std::endl;

            close(serverSocket);

            exit(-1);

        }

        std::cout << "Server listening on IPv4, port " << PORT << std::endl;

    } else if (allowedIp.contains(":")) {

        serverSocket = socket(AF_INET6, SOCK_STREAM, 0);
        if (serverSocket < 0) {

            std::cerr << "Failed to create IPv6 socket" << std::endl;

            exit(-1);

        }

        sockaddr_in6 serverAddr{};
        serverAddr.sin6_family = AF_INET6;
        serverAddr.sin6_addr = in6addr_any;
        serverAddr.sin6_port = htons(PORT);

        if (bind(serverSocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) < 0) {

            std::cerr << "Failed to bind IPv6 socket on port " << PORT << std::endl;

            close(serverSocket);

            exit(-1);

        }

        std::cout << "Server listening on IPv6, port " << PORT << std::endl;

    } else {

        std::cerr << "Invalid Supervisor-Server IP in config." << std::endl;

        exit(-1);

    }

    if (listen(serverSocket, 5) < 0) {

        std::cerr << "Failed to listen on socket" << std::endl;

        close(serverSocket);

        exit(-1);

    }

    while (keepRunning) {

        sockaddr_storage clientAddr{};
        socklen_t addrLen = sizeof(clientAddr);

        if (int clientSocket = accept(serverSocket, reinterpret_cast<sockaddr*>(&clientAddr), &addrLen); clientSocket >= 0) {

            char clientIP[INET6_ADDRSTRLEN] = {};

            if (clientAddr.ss_family == AF_INET) {

                const auto* addr = reinterpret_cast<sockaddr_in*>(&clientAddr);
                if (inet_ntop(AF_INET, &addr->sin_addr, clientIP, INET_ADDRSTRLEN) == nullptr) {

                    std::cerr << "Failed to convert IPv4 address" << std::endl;

                    close(clientSocket);

                    return;

                }

            } else if (clientAddr.ss_family == AF_INET6) {

                const auto* addr6 = reinterpret_cast<sockaddr_in6*>(&clientAddr);
                if (inet_ntop(AF_INET6, &addr6->sin6_addr, clientIP, INET6_ADDRSTRLEN) == nullptr) {

                    std::cerr << "Failed to convert IPv6 address" << std::endl;

                    close(clientSocket);

                    return;

                }

            }

            std::string clientIpStr(clientIP);
            std::thread(handleClient, clientSocket, allowedIp, clientIpStr).detach();

        } else {

            std::cerr << "Failed to accept connection" << std::endl;

        }

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

void monitorCpuUsage() {

    while (keepRunning) {

        double cpuUsage = getCpuUsage();

        json cpuStats = {
            {"timestamp", std::chrono::system_clock::to_time_t(std::chrono::system_clock::now())},
            {"cpu_usage_percent", cpuUsage}
        };

        std::lock_guard lock(file_mutex);
        if (std::ofstream file(RESULTS_PATH, std::ios::trunc); file) {

            file << cpuStats.dump(4);

        } else {

            std::cerr << "Error: Could not write to " << RESULTS_PATH << std::endl;
        }


        std::this_thread::sleep_for(std::chrono::seconds(1));

    }

}

int main() {

    std::thread serverThread(runServer);
    std::thread cpuMonitoringThread(monitorCpuUsage);

    serverThread.join();
    cpuMonitoringThread.join();

    return 0;

}
