#include "JSONManager.h"

#include <fstream>
#include <iostream>

#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/filereadstream.h>
#include <rapidjson/filewritestream.h>

void JSONManager::create_generic_file(const char* file_path)
{

    // Create JSON array
    rapidjson::Document d;
    d.SetArray();

    // Get allocator
    rapidjson::Document::AllocatorType& allocator = d.GetAllocator();

    // Create the object inside the array
    rapidjson::Value obj(rapidjson::kObjectType);

    // Add data to the JSON document
    obj.AddMember("display_name", "Localhost", allocator);
    obj.AddMember("hostname", "127.0.0.1", allocator);

    // Push the object into the array
    d.PushBack(obj, allocator);

    // Convert the document to a string
    rapidjson::StringBuffer buffer;
    rapidjson::Writer writer(buffer);
    d.Accept(writer);

    // Write the JSON string to the file
    std::ofstream outFile(file_path);
    outFile << buffer.GetString();
    outFile.close();

    std::cout << "[INF] File generated!" << std::endl;

}

void JSONManager::append_data_to_file(const char* file_path, const char* data)
{

    // Open file
    FILE* fp = fopen(file_path, "r");

    // Create document and read file's data
    rapidjson::Document d;
    if (fp)
    {

        char readBuffer[65536];
        rapidjson::FileReadStream is(fp, readBuffer, sizeof(readBuffer));

        // Parse file's data to JSON
        d.ParseStream(is);

        // Close file
        fclose(fp);

    }
    else
    {

        std::cerr << "[ERR] Could not find specified file! Exiting..." << std::endl;

        return;

    }

    // Check if the JSON is in the hosts' format
    if (!d.IsArray())
    {

        std::cerr << "[ERR] Invalid JSON format in file! Expected an array." << std::endl;

        return;

    }

    // Parse the data into a rapidjson::Value
    rapidjson::Document tempDoc;
    if (tempDoc.Parse(data).HasParseError())
    {

        std::cerr << "[ERR] Failed to parse the provided content JSON." << std::endl;

        return;

    }

    // Ensure input is an object or array
    if (!tempDoc.IsArray() && !tempDoc.IsObject())
    {

        std::cerr << "[ERR] Unexpected JSON format in data! Exiting..." << std::endl;

        return;

    }

    // Get allocator from main document
    rapidjson::Document::AllocatorType& allocator = d.GetAllocator();

    auto isDuplicate = [&](const std::string& name, const std::string& ip)
    {

        for (const auto& item : d.GetArray())
        {

            if (item.HasMember("display_name") && item["display_name"].IsString() &&

                item["display_name"].GetString() == name)

                return true;

            if (item.HasMember("hostname") && item["hostname"].IsString() &&

                item["hostname"].GetString() == ip)

                return true;

        }

        return false;

    };

    // Function to safely get string value from a Value
    auto getString = [](const rapidjson::Value& obj, const char* key) -> std::string
    {
        return obj.HasMember(key) && obj[key].IsString() ? obj[key].GetString() : "";
    };

    // Add single or multiple entries
    if (tempDoc.IsArray())
    {

        for (auto& item : tempDoc.GetArray())
        {

            std::string name = getString(item, "display_name");
            std::string ip = getString(item, "hostname");

            if (isDuplicate(name, ip))
            {

                std::cout << "[INF] Duplicate entry skipped: " << name << " / " << ip << std::endl;

                continue;

            }

            d.PushBack(item, allocator);

        }

    }
    else if (tempDoc.IsObject())
    {

        std::string name = getString(tempDoc, "display_name");
        std::string ip = getString(tempDoc, "hostname");

        if (isDuplicate(name, ip))
        {

            std::cout << "[INF] Duplicate entry skipped: " << name << " / " << ip << std::endl;

            return;

        }

        d.PushBack(tempDoc, allocator);

    }

    // Open file to write data
    FILE* outFp = fopen(file_path, "w");
    if (!outFp) {

        std::cerr << "[ERR] Failed to open the file! Exiting... " << std::endl;

        return;

    }

    // Write data
    char writeBuffer[65536];
    rapidjson::FileWriteStream os(outFp, writeBuffer, sizeof(writeBuffer));
    rapidjson::Writer writer(os);
    d.Accept(writer);
    fclose(outFp);

}