#include "Polyweb/polyweb.hpp"
#include <fstream>

int main() {
    pn::init();

    pw::Server server;
    server.bind("0.0.0.0", 8000);
    server.route("/*", [](pw::Connection& conn, const pw::HTTPRequest& req) {
        std::ifstream t("./" + req.target);
        std::string str((std::istreambuf_iterator<char>(t)),
            std::istreambuf_iterator<char>());
        try {
            return pw::HTTPResponse("200", str, {{"Content-Type", pw::filename_to_mimetype(req.target)}});
        } catch (std::exception& e) {
            return pw::HTTPResponse("200", str);
        }
    });
    server.route("/greetme/*", [](pw::Connection& conn, const pw::HTTPRequest& req) {
        pw::HTTPResponse resp("200", "<html><body><h1>Hello, " + req.target + "!</h1></body></html>", {{"Content-Type", pw::filename_to_mimetype("html")}});
        return resp;
    });
    server.listen(128);

    return pn::quit();
}