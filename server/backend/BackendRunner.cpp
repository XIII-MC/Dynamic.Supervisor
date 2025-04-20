#include "BackendRunner.h"
#include "../utils/json/JSONManager.h"

#include <crow.h>
#include <fstream>

const std::string HOSTS_PATH = "/var/lib/dynamic-supervisor/srv/hosts.json";

void BackendRunner::start_crow() {

    crow::SimpleApp app;

    CROW_ROUTE(app, "/api/v1/hosts/add").methods(crow::HTTPMethod::Post)
    ([&](const crow::request& req) {

        const std::string& body = req.body;
        if (body.empty()) {
            return crow::response(400, "Missing JSON body");
        }

        // Append host
        JSONManager::append_data_to_file(HOSTS_PATH.c_str(), body.c_str());

        return crow::response(200, "Host added");

    });

    app.bindaddr("127.0.0.1").port(18080).multithreaded().run_async();

}