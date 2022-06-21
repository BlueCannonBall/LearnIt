#include "Polyweb/polyweb.hpp"
#include <fstream>

int main() {
    pn::init();

    pw::Server server;
    server.bind("0.0.0.0", 5000);

    server.route("/*", [](pw::Connection& conn, const pw::HTTPRequest& req) {
        std:: cout << "Got request:\n" << req.build_str() << std::endl;

        std::ifstream file("./" + req.target);
        if (!file.is_open()) {
            return pw::HTTPResponse("500", "Error opening file\n", {{"Content-Type", "text/plain"}});
        }

        try {
            std::string str((std::istreambuf_iterator<char>(file)),
                std::istreambuf_iterator<char>());

            return pw::HTTPResponse("200", str, {{"Content-Type", pw::filename_to_mimetype(req.target)}});
        } catch (const std::ios::failure& e) {
            return pw::HTTPResponse("500", "Error reading file\n", {{"Content-Type", "text/plain"}});
        }

    });

    server.listen(128);

    return pn::quit();
}