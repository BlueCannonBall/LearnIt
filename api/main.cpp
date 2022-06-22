#include "Polyweb/polyweb.hpp"
#include <fstream>
#include <signal.h>

int main() {
    pn::init();
    signal(SIGPIPE, SIG_IGN);

    pw::Server server;
    if (server.bind("0.0.0.0", 5000) == PW_ERROR) {
        std::cerr << pw::universal_strerror(pw::get_last_error()) << std::endl;
        return 1;
    }

    server.route("/*", [](pw::Connection& conn, const pw::HTTPRequest& req) {
        std:: cout << "Got request:\n" << req.build_str() << std::endl;

        std::ifstream file("./" + req.target);
        if (!file.is_open()) {
            return pw::HTTPResponse("404", "Error opening file\n", {{"Content-Type", "text/plain"}});
        }

        try {
            std::string str((std::istreambuf_iterator<char>(file)),
                std::istreambuf_iterator<char>());

            return pw::HTTPResponse("200", str, {{"Content-Type", pw::filename_to_mimetype(req.target)}});
        } catch (const std::ios::failure& e) {
            return pw::HTTPResponse("500", "Error reading file\n", {{"Content-Type", "text/plain"}});
        }
    });

    server.route("/", [](pw::Connection& conn, const pw::HTTPRequest& req) {
        std:: cout << "Got request:\n" << req.build_str() << std::endl;

        std::ifstream file("index.html");
        if (!file.is_open()) {
            return pw::HTTPResponse("404", "Error opening file\n", {{"Content-Type", "text/plain"}});
        }

        try {
            std::string str((std::istreambuf_iterator<char>(file)),
                std::istreambuf_iterator<char>());

            return pw::HTTPResponse("200", str, {{"Content-Type", "text/html"}});
        } catch (const std::ios::failure& e) {
            return pw::HTTPResponse("500", "Error reading file\n", {{"Content-Type", "text/plain"}});
        }
    });

    if (server.listen(128) == PW_ERROR) {
        std::cerr << pw::universal_strerror(pw::get_last_error()) << std::endl;
        return 1;
    }

    return pn::quit();
}