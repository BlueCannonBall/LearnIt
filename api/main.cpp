#include "Polyweb/polyweb.hpp"

int main() {
    pn::init();

    pw::Server server;
    server.bind("0.0.0.0", 8000);
    server.route("/", [](pw::Connection& conn, const pw::HTTPRequest& req) {
        pw::HTTPResponse resp("200", "<html><body><h1>Hello, World!</h1></body></html>", {{"Content-Type", pw::filename_to_mimetype("html")}});
        return resp;
    });
    server.listen(128);

    return pn::quit();
}