#pragma once

#include "Command.hpp"
#include <unordered_map>
class Parser {
public:
    Parser(const std::string &filename);
    bool parse();
    const std::vector<Command> &getCommands() const;
    const std::vector<Error> &getErrors() const;
    bool hasErrors() const;
private:
    std::string filename;
    std::vector<Command> commands;
    std::vector<Error> errors;
    std::vector<Token> tokens;
    int currentTokenIndex;
    int currentLine;

    static const std::unordered_set<std::string> jump0;
    static const std::unordered_set<std::string> jump1;
    static const std::unordered_set<std::string> jump2;
    static const std::unordered_set<std::string> arithmetic1;
    static const std::unordered_set<std::string> arithmetic2;
    static const std::unordered_set<std::string> arithmetic3;
    static const std::unordered_set<std::string> keywords;
    static const std::unordered_map<std::string, size_t> specialRegisters;
    static size_t getRegisterIndex(const std::string &name);

    static constexpr size_t REGISTER_INVALID = ~0;

    bool parseCommand();
    bool parseCommandUnaligned();
    bool parseInstruction();
    bool parseInstruction0(const std::string &name);
    bool parseInstruction1(const std::string &name);
    bool parseInstruction2(const std::string &name);
    bool parseInstruction3(const std::string &name);
    bool parseDirective();
    bool parseDup();
    bool parseDD();
    bool parseSymbolDefinition();
    bool parseLabel();
    bool parseNewline();

    void addError(const std::string &message);

    bool isRegister(int offset = 0) const;
    bool isSymbol(int offset = 0) const;
    bool isNumber(int offset = 0) const;
    bool isNumberOrSymbol(int offset = 0) const;
    bool isRegisterIndirect(int offset = 0) const;
    bool isRegisterIndirectWithDisplacement(int offset = 0) const;
    bool isOperand(int offset = 0) const;

    void consumeNumber();
    void consumeSymbol();
    void consumeRegister();
    void consumeNumberOrSymbol();
    void consumeRegisterIndirect();
    void consumeRegisterIndirectWithDisplacement(); // todo: refactor number
    void consumeOperand();

    void getOperatorInfo(AddressingMode &addressingMode, std::string &numberOrSymbol, Command::Sign &sign, int &r); // todo: implement

    Token getToken(int offset) const;
    Token currentToken() const;
    Token nextToken();

    bool hasArity0(const std::string &instruction) const;
    bool hasArity1(const std::string &instruction) const;
    bool hasArity2(const std::string &instruction) const;
    bool hasArity3(const std::string &instruction) const;
};
