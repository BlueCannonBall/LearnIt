#include "Polyweb/polyweb.hpp"
#include "persistence-odb.hxx"
#include "persistence.hpp"
#include <iomanip>
#include <ios>
#include <nlohmann/json.hpp>
#include <odb/database.hxx>
#include <odb/mysql/database.hxx>
#include <odb/transaction.hxx>
#include <openssl/sha.h>
#include <signal.h>
#include <sstream>
#include <time.h>
#include <variant>
#define JWT_DISABLE_PICOJSON
#include <jwt-cpp/jwt.h>
#include <jwt-cpp/traits/nlohmann-json/traits.h>

#define ISSUER "LearnIt!"
#define SECRET "!bcbmagic12232004"
#define VERIFY(token, username)                                                                      \
    do {                                                                                             \
        try {                                                                                        \
            username = verify_token(token);                                                          \
        } catch (const jwt::token_verification_exception& e) {                                       \
            return generate_error_resp("401", std::string("Token verfication failed: ") + e.what()); \
        } catch (const jwt::error::claim_not_present_exception& e) {                                 \
            return generate_error_resp("401", std::string("Token verfication failed: ") + e.what()); \
        } catch (const std::bad_cast& e) {                                                           \
            return generate_error_resp("401", "Token verfication failed: Bad claim");                \
        }                                                                                            \
    } while (0)

using namespace odb::core;
using json = nlohmann::json;

bool is_number(const std::string& s) {
    std::string::const_iterator it = s.begin();
    while (it != s.end() && isdigit(*it)) ++it;
    return !s.empty() && it == s.end();
}

std::string sha256(const std::string& str) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256((const unsigned char*) str.data(), str.size(), hash);
    std::stringstream ss;
    ss << std::hex << std::setw(2) << std::setfill('0');
    for (unsigned char i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        ss << (unsigned int) hash[i];
    }
    return ss.str();
}

pw::HTTPResponse generate_error_resp(const std::string& status_code, const std::string& error) {
    json json_resp;
    json_resp["error"] = error;
    throw pw::HTTPResponse(status_code, json_resp.dump(), {{"Content-Type", "application/json"}});
}

std::string verify_token(const std::string& token) {
    auto decoded = jwt::decode<jwt::traits::nlohmann_json>(token);
    auto verifier = jwt::verify<jwt::traits::nlohmann_json>()
                        .allow_algorithm(jwt::algorithm::hs256 {SECRET})
                        .with_issuer(ISSUER);
    verifier.verify(decoded);
    return decoded.get_payload_claim("username").as_string();
}

int main(int argc, char** argv) {
    pn::init();
    signal(SIGPIPE, SIG_IGN);

    std::unique_ptr<database> db(new odb::mysql::database("tt", "1221", "learnit"));

    pw::Server server;
    if (server.bind("0.0.0.0", 5000) == PW_ERROR) {
        std::cerr << pw::universal_strerror() << std::endl;
        return 1;
    }

    server.route("/v1/signup", [&db](pw::Connection& conn, const pw::HTTPRequest& req) -> pw::HTTPResponse {
        json json_req;
        try {
            json_req = json::parse(req.body);
        } catch (const std::exception& e) {
            return generate_error_resp("400", std::string("Invalid JSON: ") + std::string(e.what()));
        }

        if (!json_req.is_object()) {
            return generate_error_resp("400", "Request is not an object");
        }
        if (json_req.find("username") == json_req.end()) {
            return generate_error_resp("400", "\"username\" field not found");
        }
        if (!json_req["username"].is_string()) {
            return generate_error_resp("400", "\"username\" is not a string");
        }
        if (json_req.find("email") == json_req.end()) {
            return generate_error_resp("400", "\"email\" field not found");
        }
        if (!json_req["email"].is_string()) {
            return generate_error_resp("400", "\"email\" is not a string");
        }
        if (json_req.find("password") == json_req.end()) {
            return generate_error_resp("400", "\"password\" field not found");
        }
        if (!json_req["password"].is_string()) {
            return generate_error_resp("400", "\"password\" is not a string");
        }

        User new_user;
        new_user.username = json_req["username"];
        new_user.email = json_req["email"];
        new_user.password_hash = sha256(json_req["password"]);
        new_user.creation_date = time(NULL);

        session s;

        try {
            transaction t(db->begin());
            db->persist(new_user);
            t.commit();
        } catch (const std::exception& e) {
            return generate_error_resp("400", e.what());
        }

        json json_resp;
        auto token = jwt::create<jwt::traits::nlohmann_json>()
                         .set_issuer(ISSUER)
                         .set_type("JWS")
                         .set_payload_claim("username", json_req["username"])
                         .sign(jwt::algorithm::hs256 {SECRET});
        json_resp["token"] = token;

        return pw::HTTPResponse("200", json_resp.dump(), {{"Content-Type", "application/json"}});
    });

    server.route("/v1/login", [&db](pw::Connection& conn, const pw::HTTPRequest& req) -> pw::HTTPResponse {
        json json_req;
        try {
            json_req = json::parse(req.body);
        } catch (const std::exception& e) {
            return generate_error_resp("400", std::string("Invalid JSON: ") + std::string(e.what()));
        }

        if (!json_req.is_object()) {
            return generate_error_resp("400", "Request is not an object");
        }
        if (json_req.find("username") == json_req.end()) {
            return generate_error_resp("400", "\"username\" field not found");
        }
        if (!json_req["username"].is_string()) {
            return generate_error_resp("400", "\"username\" is not a string");
        }
        if (json_req.find("password") == json_req.end()) {
            return generate_error_resp("400", "\"password\" field not found");
        }
        if (!json_req["password"].is_string()) {
            return generate_error_resp("400", "\"password\" is not a string");
        }

        session s;

        std::shared_ptr<User> user;
        try {
            transaction t(db->begin());
            user = db->load<User>(json_req["username"]);
            t.commit();
        } catch (const std::exception& e) {
            return generate_error_resp("400", e.what());
        }

        if (sha256(json_req["password"]) != user->password_hash) {
            return generate_error_resp("401", "Authentication failed: Incorrect password");
        }

        json json_resp;
        auto token = jwt::create<jwt::traits::nlohmann_json>()
                         .set_issuer(ISSUER)
                         .set_type("JWS")
                         .set_payload_claim("username", json_req["username"])
                         .sign(jwt::algorithm::hs256 {SECRET});
        json_resp["token"] = token;

        return pw::HTTPResponse("200", json_resp.dump(), {{"Content-Type", "application/json"}});
    });

    server.route("/v1/create_deck", [&db](pw::Connection& conn, const pw::HTTPRequest& req) -> pw::HTTPResponse {
        json json_req;
        try {
            json_req = json::parse(req.body);
        } catch (const std::exception& e) {
            return generate_error_resp("400", std::string("Invalid JSON: ") + std::string(e.what()));
        }

        if (!json_req.is_object()) {
            return generate_error_resp("400", "Request is not an object");
        }
        if (json_req.find("token") == json_req.end()) {
            return generate_error_resp("400", "\"token\" field not found");
        }
        if (!json_req["token"].is_string()) {
            return generate_error_resp("400", "\"token\" is not a string");
        }
        if (json_req.find("name") == json_req.end()) {
            return generate_error_resp("400", "\"name\" field not found");
        }
        if (!json_req["name"].is_string()) {
            return generate_error_resp("400", "\"name\" is not a string");
        }
        if (json_req.find("terms") == json_req.end()) {
            return generate_error_resp("400", "\"terms\" field not found");
        }
        if (!json_req["terms"].is_object()) {
            return generate_error_resp("400", "\"terms\" is not an object");
        }

        std::string username;
        VERIFY(json_req["token"], username);

        session s;

        std::shared_ptr<User> user;
        try {
            transaction t(db->begin());
            user = db->load<User>(username);
            t.commit();
        } catch (const std::exception& e) {
            return generate_error_resp("400", e.what());
        }

        std::shared_ptr<Deck> new_deck(new Deck);
        new_deck->owner = user;
        new_deck->name = json_req["name"];
        new_deck->creation_date = time(NULL);
        for (const auto& term : json_req["terms"].items()) {
            if (!term.value().is_string()) {
                return generate_error_resp("400", "Term definition is not a string");
            }

            try {
                std::shared_ptr<Term> new_term(new Term(term.key(), term.value()));
                transaction t(db->begin());
                db->persist(new_term);
                t.commit();
                new_deck->terms.push_back(std::move(new_term));
            } catch (const std::exception& e) {
                generate_error_resp("500", e.what());
            }
        }

        unsigned long long new_deck_id;
        try {
            transaction t(db->begin());
            new_deck_id = db->persist(new_deck);
            user->decks.push_back(new_deck);
            db->update(user);
            t.commit();
        } catch (const std::exception& e) {
            generate_error_resp("500", e.what());
        }

        json json_resp;
        json_resp["id"] = new_deck_id;

        return pw::HTTPResponse("200", json_resp.dump(), {{"Content-Type", "application/json"}});
    });

    server.route("/v1/deck_info/*", [&db](pw::Connection& conn, const pw::HTTPRequest& req) -> pw::HTTPResponse {
        std::vector<std::string> split_target;
        boost::split(split_target, req.target, boost::is_any_of("/"));

        if (!is_number(split_target.back())) {
            return generate_error_resp("400", "Deck ID is not a number");
        }
        unsigned long long deck_id = std::stoull(split_target.back());

        session s;

        std::shared_ptr<Deck> deck;
        try {
            transaction t(db->begin());
            deck = db->load<Deck>(deck_id);
            deck->owner.load();
            t.commit();
        } catch (const std::exception& e) {
            return generate_error_resp("400", e.what());
        }

        json json_resp;
        json_resp["name"] = deck->name;
        json_resp["owner"] = deck->owner.load()->username;
        json_resp["creation_date"] = deck->creation_date;
        json_resp["terms"] = json::object();
        for (const auto& term : deck->terms) {
            json_resp["terms"][term->term] = term->definition;
        }

        return pw::HTTPResponse("200", json_resp.dump(), {{"Content-Type", "application/json"}});
    });

    server.route("/v1/user_info/*", [&db](pw::Connection& conn, const pw::HTTPRequest& req) -> pw::HTTPResponse {
        std::vector<std::string> split_target;
        boost::split(split_target, req.target, boost::is_any_of("/"));
        std::string username = std::move(split_target.back());

        session s;

        std::shared_ptr<User> user;
        try {
            transaction t(db->begin());
            user = db->load<User>(username);
            t.commit();
        } catch (const std::exception& e) {
            return generate_error_resp("400", e.what());
        }

        json json_resp;
        json_resp["username"] = user->username;
        json_resp["creation_date"] = user->creation_date;
        json_resp["decks"] = json::array();
        for (const auto& deck : user->decks) {
            transaction t(db->begin());
            json_resp["decks"].push_back(deck.load()->id);
            t.commit();
        }

        return pw::HTTPResponse("200", json_resp.dump(), {{"Content-Type", "application/json"}});
    });

    if (server.listen(128) == PW_ERROR) {
        std::cerr << pw::universal_strerror() << std::endl;
        return 1;
    }

    return pn::quit();
}