#include "utils/json/JSONManager.h"
#include "backend/BackendRunner.h"

#include <fstream>
#include <iostream>

#include <rapidjson/document.h>
#include <rapidjson/filereadstream.h>

const std::string HOSTS_PATH = "/var/lib/dynamic-supervisor/srv/hosts.json";

int main()
{

    // Start server (Dynamic.Supervisor-SRV)

    // Read hosts.json file containing hosts information and process it:
    // Read and extract part:

    // Open file and check if it exists, if not, generate a "generic" one
    FILE* file = nullptr;
    int try_count = 0;
    while ((file = fopen(HOSTS_PATH.c_str(), "r")) == nullptr && try_count++ <= 1)
    {

        std::cerr << "[ERR] Could not find specified file! "
        << (try_count <= 1 ? "Generating a generic one... " : "Exiting...")
        << std::endl;

        // Generate a generic hosts file
        JSONManager::create_generic_file(HOSTS_PATH.c_str());

    }

    // Read file
    char readBuffer[65536];
    rapidjson::FileReadStream is(file, readBuffer, sizeof(readBuffer));

    // Parse file to JSON
    rapidjson::Document d;
    d.ParseStream(is);
    fclose(file);

    // Check if the JSON is in the hosts' format
    if (!d.IsArray())
    {

        std::cerr << "[ERR] Unexpected JSON format! Generating a generic one..." << std::endl;

        // Close file incase its being read
        fclose(file);

        // Make a backup because we are not entirely stupid...
        std::ifstream  src(HOSTS_PATH.c_str(), std::ios::binary);
        std::ofstream  dst((HOSTS_PATH + ".bkp").c_str(),   std::ios::binary);

        // Write file to backup (copy sorta)
        dst << src.rdbuf();

        // Delete the bad file
        std::remove(HOSTS_PATH.c_str());

        // Generate a generic hosts file
        JSONManager::create_generic_file(HOSTS_PATH.c_str());

    }

    // Go through all the hosts' data
    for (const auto& item : d.GetArray())
    {

        std::cout << std::endl;
        std::cout << "Display Name: " << item["display_name"].GetString() << std::endl;
        std::cout << "Hostname: " << item["hostname"].GetString() << std::endl;

    }

    BackendRunner::start_crow();

    // Since we're supposed to run forever this is not supposed to ever trigger
    return -100;

}