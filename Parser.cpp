#include "Parser.hpp"
#include "Token.hpp"
#include <iostream>
#include <unordered_set>
#include <unordered_map>
#include <algorithm>

// Categorizing instructions by type and arity
const std::unordered_set<std::string> Parser::jump0 = { "ret" };
const std::unordered_set<std::string> Parser::jump1 = { "jmp", "call" };
const std::unordered_set<std::string> Parser::jump2 = { "jz", "jnz", "jlz", "jlez", "jgz", "jgez" };

const std::unordered_set<std::string> Parser::arithmetic1 = { "inc", "dec", "neg", "push", "pop"};
const std::unordered_set<std::string> Parser::arithmetic2 = {};
const std::unordered_set<std::string> Parser::arithmetic3 = { "add", "sub", "mul" };

const std::unordered_map<std::string, size_t> Parser::specialRegisters = {
    {"zero", 0},
    {"ra", 1},
    {"sp", 2},
    {"gp", 3},
    {"fp", 8},
    {"s0", 8},
    {"s1", 9},
    {"a0", 10},
    {"a1", 11},
    {"a2", 12},
    {"a3", 13},
    {"a4", 14},
    {"a5", 15},
    {"t0", 5}, 
    {"t1", 6}, 
    {"t2", 7}, 
    {"t3", 28},
    {"t4", 29},
    {"t5", 30},
    {"t6", 31},
    {"ivtp", 26},
    {"imr", 27}
};

size_t Parser::getRegisterIndex(const std::string &name) {
    std::string normalized = name;
    std::transform(normalized.begin(), normalized.end(), normalized.begin(), [](unsigned char c) {
        return std::tolower(c);
        });
    auto it = specialRegisters.find(normalized);
    if (it != specialRegisters.end()) {
        return it->second;
    }
    if (normalized[0] == 'r' && normalized.size() > 1) {
        try {
            int regIndex = std::stoi(normalized.substr(1));
            if (regIndex >= 0 && regIndex <= 31) {
                return static_cast<size_t>(regIndex);
            }
        }
        catch (const std::exception &) {
            return REGISTER_INVALID;
        }
    }
    return REGISTER_INVALID;
}

const std::unordered_set<std::string> Parser::keywords = { "def", "include", "start", "org", "dd", "dup"};

Parser::Parser(const std::string &filename) : filename(filename), currentTokenIndex(0), currentLine(1) {
    Tokenizer tokenizer(filename);
    if (!tokenizer.tokenize()) {
        errors = tokenizer.getErrors();
    }
    else {
        tokens = tokenizer.getTokens();
    }
}

bool Parser::parse() {
    while (currentTokenIndex < tokens.size()) {
        parseCommand();
    }
    return errors.empty();
}

const std::vector<Command> &Parser::getCommands() const {
    return commands;
}

const std::vector<Error> &Parser::getErrors() const {
    return errors;
}

bool Parser::hasErrors() const {
    return !errors.empty();
}

bool Parser::parseCommand() {
    if (parseCommandUnaligned()) {
        return true;
    }
    while (!currentToken().value.empty() && currentToken().value != "\n") {
        nextToken();
    }
    nextToken(); // skip "\n"
    return false;
}

bool Parser::parseCommandUnaligned() {
    Token token = currentToken();
    if (token.value == "\n") {
        nextToken();
        return true; // skip empty line
    }
    if (token.type != Token::Type::Name) {
        addError("Command can't start with " + token.value);
        return false;
    }
    if (keywords.count(token.value)) {
        return parseDirective();
    }
    if (getToken(1).value == "def") {
        return parseSymbolDefinition();
    }
    if (getToken(1).type == Token::Type::Symbol && getToken(1).value == ":") {
        return parseLabel();
    }
    return parseInstruction();
}

bool Parser::parseInstruction() {
    Token token = currentToken();

    std::string name = token.value;
    nextToken(); // skip the name

    if (hasArity0(name)) {
        return parseInstruction0(name);
    }
    else if (hasArity1(name)) {
        return parseInstruction1(name);
    }
    else if (hasArity2(name)) {
        return parseInstruction2(name);
    }
    else if (hasArity3(name)) {
        return parseInstruction3(name);
    }
    else {
        addError("Unknown instruction: " + name);
        return false;
    }
    return parseNewline();
}

bool Parser::parseInstruction0(const std::string &name) {
    commands.push_back(Command::createInstruction0(name));
    return true;
}

bool Parser::parseInstruction1(const std::string &name) {
    if (!isOperand()) {
        addError("Expected an operand for " + name);
        return false;
    }
    if (arithmetic1.count(name) > 0 && !isRegister()) {
        addError("Invalid operand for " + name + ": " + currentToken().value + ", expected register");
        return false;
    }
    AddressingMode mode;
    if (isRegister()) {

    }
    AddressingMode addressingMode;
    std::string numberOrSymbol;
    Command::Sign sign;
    int r;
    getOperatorInfo(addressingMode, numberOrSymbol, sign, r);

    Command command = Command::createInstruction(name, addressingMode, numberOrSymbol, sign, r);
    commands.push_back(command);
    consumeOperand();
    return true;
}

bool Parser::parseInstruction2(const std::string &name) {
    AddressingMode addressingMode;
    std::string numberOrSymbol;
    Command::Sign sign;
    int r1, r2;

    if (!isOperand()) {
        addError("Expected an operand for " + name);
        return false;
    }
    if (!isRegister()) {
        addError("First operand for " + name + " must be a register");
        return false;
    }
    r1 = getRegisterIndex(currentToken().value);
    consumeOperand();
    if (!currentToken().matchToken(Token::Type::Symbol, ",")) {
        addError("Expected ',' after first operand for " + name);
        return false;
    }
    nextToken(); // skip comma
    if (!isOperand()) {
        addError("Expected an operand for " + name);
        return false;
    }
    if (name == "store" && currentToken().value == "#") {
        addError("Can't store to a constant");
        return false;
    }
    getOperatorInfo(addressingMode, numberOrSymbol, sign, r2);
    commands.push_back(Command::createInstruction(name, addressingMode, numberOrSymbol, sign, r1, r2));
    consumeOperand();
    return true;
}

bool Parser::parseInstruction3(const std::string &name) {
    int r[3];
    for (int i = 0; i < 3; ++i) {
        if (!isRegister()) {
            addError("Invalid operand for " + name + ": " + currentToken().value + ", expected register");
            return false;
        }
        r[i] = getRegisterIndex(currentToken().value);
        consumeRegister();

        if (i < 2 && !currentToken().matchToken(Token::Type::Symbol, ",")) {
            addError("Expected ',' between operands for " + name);
            return false;
        }
        nextToken(); // skip comma
    }
    commands.push_back(Command::createInstruction3(name, r[0], r[1], r[2]));
    return true;
}

bool Parser::parseDup() {
    std::string value;
    size_t repetition;
    nextToken(); // skip (
    if (!isNumberOrSymbol()) {
        addError("Expected number or symbol");
        return false;
    }
    value = currentToken().value;
    consumeNumberOrSymbol();
    if (currentToken().value != "dup") {
        addError("Invalid format for 'dd', expected 'dup'");
        return false;
    }
    nextToken(); // skip dup
    if (!isNumber()) {
        addError("Expected number");
        return false;
    }
    repetition = static_cast<uint32_t>(std::stoul(currentToken().value, nullptr, 16));
    consumeNumber();
    if (currentToken().value != ")") {
        addError("Expected ')'");
        return false;
    }
    nextToken(); // )
    commands.push_back(Command::createDirective("dup", value, repetition));
    return true;
}

bool Parser::parseDD() {
    if (currentToken().value == "(") {
        return parseDup();
    }
    std::vector<std::string> operands;
    if (!isNumberOrSymbol()) {
        addError("Expected number or symbol");
        return false;
    }
    operands.push_back(currentToken().value);
    consumeNumberOrSymbol();
    while (true) {
        if (currentToken().value != ",") {
            break;
        }
        nextToken();  // skip commas
        if (!isNumberOrSymbol()) {
            addError("Expected number or symbol");
            return false;
        }
        operands.push_back(currentToken().value);
        consumeNumberOrSymbol();
    }
    bool front = true;
    for (const std::string &operand : operands) {
        commands.push_back(Command::createDirective("dd", operand));
    }
    return parseNewline();
}

bool Parser::parseDirective() {
    Token token = currentToken();
    std::string directiveName = token.value;
    nextToken();

    if (directiveName == "include") {
        if (!isSymbol()) {
            addError("Expected filename after 'include' directive");
            return false;
        }
        commands.push_back(Command::createDirective(directiveName, currentToken().value));
        nextToken(); // skip filename
        return parseNewline();
    }
    else if (directiveName == "start" || directiveName == "org") {
        if (!isNumberOrSymbol()) {
            addError("Expected number for '" + directiveName + "'");
            return false;
        }
        commands.push_back(Command::createDirective(directiveName, currentToken().value));
        consumeNumberOrSymbol();
        return parseNewline();
    }
    else if (directiveName == "dd") {
        return parseDD();
    }

    addError("Unknown directive: " + directiveName);
    return false;
}


bool Parser::parseSymbolDefinition() {
    std::string symbol = currentToken().value;
    nextToken();  // Skip symbol
    nextToken();  // Skip 'def'
    if (!isNumberOrSymbol()) {
        addError("Expected number or symbol after 'def'");
        return false;
    }
    std::string value = currentToken().value;
    consumeNumberOrSymbol();
    commands.push_back(Command::createSymbolDefinition(symbol, value));
    return parseNewline();
}

bool Parser::parseNewline() {
    if (currentToken().value.empty() || currentToken().value != "\n") {
        addError("Only one command per line");
        return false;
    }
    nextToken(); // skip "\n"
    return true;
}

bool Parser::parseLabel() {
    Token labelToken = currentToken();
    nextToken();  // Skip label
    nextToken();  // Skip ':'
    commands.push_back(Command::createLabel(labelToken.value));
    return true;
}

void Parser::addError(const std::string &message) {
    errors.emplace_back(message, currentLine, filename);
}

Token Parser::getToken(int offset) const {
    int index = currentTokenIndex + offset;
    if (index < tokens.size()) {
        return tokens[index];
    }
    return { Token::Type::Unknown, "" };
}

Token Parser::currentToken() const {
    return getToken(0);
}

Token Parser::nextToken() {
    currentTokenIndex++;
    return currentToken();
}

bool Parser::isRegister(int offset) const {
    return getRegisterIndex(getToken(offset).value) != REGISTER_INVALID;
}

bool Parser::isSymbol(int offset) const {
    return 
        getToken(offset).matchToken(Token::Type::Name) && 
        keywords.find(getToken(offset).value) == keywords.end() && 
        !isRegister(offset);
}

bool Parser::isNumber(int offset) const {
    return getToken(offset).matchToken(Token::Type::Number);
}

bool Parser::isNumberOrSymbol(int offset) const {
    return isSymbol(offset) || isNumber(offset);
}

bool Parser::isRegisterIndirect(int offset) const {
    return
        !getToken(offset).matchToken(Token::Type::Symbol, "[") &&
        !isRegister(offset + 1) &&
        !getToken(offset + 2).matchToken(Token::Type::Symbol, "]");
}

bool Parser::isRegisterIndirectWithDisplacement(int offset) const {
    int currentDisplacement = offset;
    if (!getToken(currentDisplacement).matchToken(Token::Type::Symbol, "[")) {
        return false;
    }

    currentDisplacement++;

    if (!isRegister(currentDisplacement) && !isNumberOrSymbol(currentDisplacement)) {
        return false;
    }
    bool operand1IsRegister = isRegister(currentDisplacement);
    currentDisplacement++;

    std::string operand = getToken(currentDisplacement).value;
    if (operand != "+" && operand != "-") {
        return false;
    }
    currentDisplacement++;

    if (!isRegister(currentDisplacement) && !isNumberOrSymbol(currentDisplacement)) {
        return false;
    }
    bool operand2IsRegister = isRegister(currentDisplacement);
    currentDisplacement++;

    if (!getToken(currentDisplacement).matchToken(Token::Type::Symbol, "]")) {
        return false;
    }

    // Check for illegal cases:
    if (operand1IsRegister && operand2IsRegister) {
        return false; // both are registers
    }
    if (!operand1IsRegister && !operand2IsRegister) {
        return false; // both are displacements
    }
    if (operand == "-" && operand2IsRegister) {
        return false; // register is being subtracted
    }

    return true;
}

bool Parser::isOperand(int offset) const {
    return isRegister(offset) || isNumberOrSymbol(offset) || isRegisterIndirect(offset) || isRegisterIndirectWithDisplacement(offset);
}

void Parser::consumeNumber() {
    if (currentToken().value == "#") {
        currentTokenIndex += 2;
    }
    else {
        currentTokenIndex++;
    }
}

void Parser::consumeSymbol() {
    currentTokenIndex++;
}

void Parser::consumeRegister() {
    currentTokenIndex++;
}

void Parser::consumeNumberOrSymbol() {
    if (isNumber()) {
        consumeNumber();
    }
    else {
        consumeSymbol();
    }
}

void Parser::consumeRegisterIndirect() {
    currentTokenIndex += 3;
}

void Parser::consumeRegisterIndirectWithDisplacement() {
    currentTokenIndex += 5;
}

void Parser::consumeOperand() {
    if (isRegister()) {
        consumeRegister();
    }
    else if (isNumberOrSymbol()) {
        consumeNumberOrSymbol();
    }
    else if (isRegisterIndirect()) {
        consumeRegisterIndirect();
    }
    else if (isRegisterIndirectWithDisplacement()) {
        consumeRegisterIndirectWithDisplacement();
    }
}

void Parser::getOperatorInfo(AddressingMode &addressingMode, std::string &numberOrSymbol, Command::Sign &sign, int &r) {
    sign = Command::Sign::Plus;
    numberOrSymbol = "";
    r = REGISTER_INVALID;
    if (isRegister()) {
        addressingMode = AddressingMode::RegisterDirect;
        r = getRegisterIndex(getToken(0).value);
    }
    else if (isNumberOrSymbol()) {
        addressingMode = AddressingMode::MemoryDirect;
        numberOrSymbol = getToken(0).value;
    }
    else if (isRegisterIndirect()) {
        addressingMode = AddressingMode::RegisterIndirect;
        r = getRegisterIndex(getToken(1).value);
    }
    else if (isRegisterIndirectWithDisplacement()) {
        addressingMode = AddressingMode::RegisterIndirectWithDisplacement;
        if (isRegister(1)) {
            r = getRegisterIndex(getToken(1).value);
            numberOrSymbol = getToken(3).value;
            sign = getToken(2).value == "+" ? Command::Sign::Plus : Command::Sign::Minus;
        }
        else {
            r = getRegisterIndex(getToken(3).value);
            numberOrSymbol = getToken(1).value;
        }
    }
}

bool Parser::hasArity0(const std::string &instruction) const {
    return jump0.count(instruction) > 0;
}

bool Parser::hasArity1(const std::string &instruction) const {
    return jump1.count(instruction) > 0 || arithmetic1.count(instruction) > 0;
}

bool Parser::hasArity2(const std::string &instruction) const {
    return jump2.count(instruction) > 0 || arithmetic2.count(instruction) > 0 || instruction == "load" || instruction == "store";
}

bool Parser::hasArity3(const std::string &instruction) const {
    return arithmetic3.count(instruction) > 0;
}
