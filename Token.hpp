#pragma once

#include <string>
#include <vector>
#include "Error.hpp"

struct Token {
    enum class Type {
        Name,
        Number,
        Symbol,
        Unknown
    };
    Type type;
    std::string value;

    Token(Type type, const std::string &value) : type(type), value(value) {}

    bool matchToken(Token::Type expectedType, const std::string &expectedValue = "");
};

class Tokenizer {
public:
    Tokenizer(const std::string &filename);
    bool tokenize();
    const std::vector<Token> &getTokens() const;
    const std::vector<Error> &getErrors() const;
    bool hasErrors() const;
private:
    static const std::vector<std::string> allowedSymbols;
    std::vector<Token> tokens;
    std::vector<Error> errors;
    std::string fileContent;
    int currentLine;

    void addToken(Token token);
    void addError(const std::string &message);
    bool isNameStart(char c) const;
    bool isNamePart(char c) const;
    bool isDigit(char c) const;
    bool isHexDigit(char c) const;
    bool isWhitespace(char c) const;
    bool isSymbol(char c) const;
    void skipWhitespaceAndComments(const std::string &content, size_t &i);
    std::string parseNumber(const std::string &content, size_t &i);
    std::string parseName(const std::string &content, size_t &i);
    std::string parseSymbol(const std::string &content, size_t &i);
    void tokenizeFile(const std::string &content);
};
