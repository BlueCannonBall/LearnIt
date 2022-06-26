#pragma once

#include <memory>
#include <odb/core.hxx>
#include <odb/lazy-ptr.hxx>
#include <string>
#include <vector>

class Deck;

#pragma db object
class User {
private:
    friend class odb::access;

public:
#pragma db id
    std::string username;
    std::string email;
    std::string password_hash;
    time_t creation_date;
#pragma db value_not_null unordered inverse(owner)
    std::vector<odb::lazy_weak_ptr<Deck>> decks;

    User() {};
};

class Term;

#pragma db object
class Deck {
private:
    friend class odb::access;

public:
#pragma db id auto
    unsigned long long id;
#pragma db not_null
    std::shared_ptr<User> owner;
    std::string name;
    time_t creation_date;
#pragma db value_not_null
    std::vector<odb::lazy_shared_ptr<Term>> terms;

    Deck() {};
};

#pragma db object
class Term {
private:
    friend class odb::access;

public:
#pragma db id auto
    unsigned long long id;
    std::string term;
    std::string definition;

    Term() {};
    Term(const std::string& term, const std::string& definition) :
        term(term),
        definition(definition) { }
};