#include <odb/core.hxx>
#include <string>

#pragma db object
class User {
private:
    friend class odb::access;

public:
    #pragma db id auto
    unsigned long long id;

    std::string name;
    std::string password_hash;
    time_t creation_date;

    User() = default;
};

#pragma db object
class Deck {
private:
    friend class odb::access;

public:
    #pragma db id auto
    unsigned long long id;

    unsigned long long owner;
    std::string name;
    time_t creation_date;

    Deck() = default;
};

#pragma db object
class Term {
private:
    friend class odb::access;

public:
    #pragma db id auto
    unsigned long long id;
    
    unsigned long long owner;
    std::string term;
    std::string definition;

    Term() = default;
};