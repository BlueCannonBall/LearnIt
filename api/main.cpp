#include "Polyweb/polyweb.hpp"
#include "persistence-odb.hxx"
#include "persistence.hpp"
#include <iomanip>
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
#define VERIFY(token)                                                                                \
    do {                                                                                             \
        try {                                                                                        \
            verify_token(token);                                                                     \
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

std::string sha256(const std::string& str) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, str.c_str(), str.size());
    SHA256_Final(hash, &sha256);
    std::stringstream ss;
    for (unsigned char i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int) hash[i];
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

    server.route("/v1/signup", [&db](pw::Connection& conn, const pw::HTTPRequest& req) {
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

    server.route("/v1/login", [&db](pw::Connection& conn, const pw::HTTPRequest& req) {
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

        std::unique_ptr<User> user;
        try {
            transaction t(db->begin());
            user = std::unique_ptr<User>(db->load<User>(json_req["username"]));
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

    server.route("/v1/create_deck", [&db](pw::Connection& conn, const pw::HTTPRequest& req) {
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
        VERIFY(json_req["token"]);

        session s;

        std::shared_ptr<User> user;
        try {
            transaction t(db->begin());
            user = std::shared_ptr<User>(db->load<User>(username));
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

            std::shared_ptr<Term> new_term(new Term(term.key(), term.value()));
            transaction t(db->begin());
            db->persist(*new_term);
            t.commit();
            new_deck->terms.push_back(std::move(new_term));
        }

        unsigned long long new_deck_id;
        try {
            transaction t(db->begin());
            new_deck_id = db->persist(*new_deck);
            user->decks.push_back(new_deck);
            db->update(*user);
            t.commit();
        } catch (const std::exception& e) {
            generate_error_resp("400", e.what());
        }

        json json_resp;
        json_resp["id"] = new_deck_id;

        return pw::HTTPResponse("200", json_resp.dump(), {{"Content-Type", "application/json"}});
    });

    if (server.listen(128) == PW_ERROR) {
        std::cerr << pw::universal_strerror() << std::endl;
        return 1;
    }

    return pn::quit();
}