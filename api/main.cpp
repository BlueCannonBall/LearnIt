#include "Polyweb/polyweb.hpp"

int main() {
    pw::Server server;
    server.bind("0.0.0.0", 8000);
    server.route("/", [](pw::Connection& conn, const pw::HTTPRequest& req) {
        auto data = req.build();
        std::cout << std::string(data.begin(), data.end()) << std::endl;
        pw::HTTPResponse resp;
        resp.status_code = "200";
        resp.reason_phrase = "OK";
        resp.headers["content-type"] = "text/html";
        std::string resp_string = "<html><body><h1>hello</h1></body></html>";
        resp.body = std::vector<char>(resp_string.begin(), resp_string.end());
        return resp;
    });
    server.listen(128);
}