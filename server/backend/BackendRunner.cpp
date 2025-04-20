#include "crow_all.h"

int main() {
    crow::SimpleApp app;

    // POST /add-host
    CROW_ROUTE(app, "/add-host").methods("POST"_method)([](const crow::request& req){
        auto body = req.body.c_str(); // JSON string sent from frontend

        JSONManager manager;
        manager.append_data_to_file("hosts.json", body);

        return crow::response(200, "Host added!");
    });

    // Serve frontend
    CROW_ROUTE(app, "/")([](){
        std::ifstream file("index.html");
        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    });

    app.port(18080).multithreaded().run();
}
