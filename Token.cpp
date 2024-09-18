#include "Token.hpp"
#include <fstream>
#include <sstream>
#include <cctype>
#include <iomanip>

bool Token::matchToken(Token::Type expectedType, const std::string &expectedValue) {
    return type == expectedType && (expectedValue.empty() || value == expectedValue);
}

const std::vector<std::string> Tokenizer::allowedSymbols = {
    "[", "]", "-", "+", ":", ",", "#", "(", ")"
};

Tokenizer::Tokenizer(const std::string &filename) : currentLine(1) {
    // Read the file content into fileContent
    std::ifstream fileStream(filename);
    if (!fileStream) {
        addError("Failed to open file");
        return;
    }
    std::stringstream buffer;
    buffer << fileStream.rdbuf();
    fileContent = buffer.str();
}

bool Tokenizer::tokenize() {
    tokenizeFile(fileContent);
    return !hasErrors();
}

const std::vector<Token> &Tokenizer::getTokens() const {
    return tokens;
}

const std::vector<Error> &Tokenizer::getErrors() const {
    return errors;
}

bool Tokenizer::hasErrors() const {
    return !errors.empty();
}

void Tokenizer::addToken(Token token) {
    tokens.push_back(token);
}

void Tokenizer::addError(const std::string &message) {
    errors.emplace_back(message, currentLine);
}

bool Tokenizer::isNameStart(char c) const {
    return std::isalpha(c) || c == '_';
}

bool Tokenizer::isNamePart(char c) const {
    return std::isalnum(c) || c == '_';
}

bool Tokenizer::isDigit(char c) const {
    return std::isdigit(c);
}

bool Tokenizer::isHexDigit(char c) const {
    return std::isxdigit(c);
}

bool Tokenizer::isWhitespace(char c) const {
    return std::isspace(c);
}

bool Tokenizer::isSymbol(char c) const {
    // Check if a character is the start of any allowed symbol
    for (const std::string &sym : allowedSymbols) {
        if (sym[0] == c) {
            return true;
        }
    }
    return false;
}

void Tokenizer::skipWhitespaceAndComments(const std::string &content, size_t &i) {
    while (i < content.size()) {
        if (isWhitespace(content[i])) {
            if (content[i] == '\n') {
                addToken({ Token::Type::Symbol, "\n" });
                ++currentLine;
            }
            ++i;
        }
        else if (content[i] == ';') {
            // Skip until the end of the line (comment)
            while (i < content.size() && content[i] != '\n') {
                ++i;
            }
        }
        else {
            break;
        }
    }
}

std::string Tokenizer::parseNumber(const std::string &content, size_t &i) {
    std::string number;
    if (content[i] == '0' && i + 1 < content.size() && content[i + 1] == 'x') {
        number += "0x";
        i += 2;
        while (i < content.size() && isHexDigit(content[i])) {
            number += content[i++];
        }
    }
    else {
        while (i < content.size() && isDigit(content[i])) {
            number += content[i++];
        }
        // Convert number to hexadecimal (2's complement)
        unsigned value = (unsigned) std::stoull(number);
        std::stringstream ss;
        ss << "0x" << std::hex << std::setw(8) << std::setfill('0') << value;
        number = ss.str();
    }
    return number;
}

std::string Tokenizer::parseName(const std::string &content, size_t &i) {
    std::string name;
    while (i < content.size() && isNamePart(content[i])) {
        name += content[i++];
    }
    return name;
}

std::string Tokenizer::parseSymbol(const std::string &content, size_t &i) {
    for (const std::string &symbol : allowedSymbols) {
        if (content.substr(i, symbol.size()) == symbol) {
            i += symbol.size();
            return symbol;
        }
    }
    return std::string(1, content[i++]);  // Fallback: treat as single character symbol
}

void Tokenizer::tokenizeFile(const std::string &content) {
    size_t i = 0;
    while (i < content.size()) {
        skipWhitespaceAndComments(content, i);

        if (i >= content.size()) {
            break;
        }

        if (isNameStart(content[i])) {
            std::string name = parseName(content, i);
            addToken({ Token::Type::Name, name });
        }
        else if (isDigit(content[i])) {
            std::string number = parseNumber(content, i);
            addToken({ Token::Type::Number, number });
        }
        else if (isSymbol(content[i])) {
            std::string symbol = parseSymbol(content, i);
            addToken({ Token::Type::Symbol, symbol });
        }
        else {
            // Unknown token
            addError("Unknown token");
            ++i;
        }
    }
}
