#include "Command.hpp"
#include <sstream>
#include <string>
#include <unordered_map>

Command Command::createInstruction0(const std::string &name) {
    return Command(Type::Instruction, name);
}

Command Command::createInstruction(const std::string &name, AddressingMode addressingMode, 
    const std::string &numberOrSymbol, Sign sign, int r1, int r2) {
    return Command(Type::Instruction, name, addressingMode, numberOrSymbol, sign, r1, r2);
}

Command Command::createInstruction3(const std::string &name, int r1, int r2, int r3) {
    return Command(Type::Instruction, name, AddressingMode::RegisterDirect, "", Sign::Plus, r1, r2, r3);
}

Command Command::createDirective(const std::string &name, const std::string &numberOrSymbol, 
    uint32_t dupNumber) {
    Command command(Type::Directive, name);
    command.numberOrSymbol = numberOrSymbol;
    command.dupNumber = dupNumber;
    return command;
}

Command Command::createSymbolDefinition(const std::string &name, const std::string &number) {
    Command command(Type::SymbolDefinition, name);
    command.numberOrSymbol = number;
    return command;
}

Command Command::createLabel(const std::string &name) {
    return Command(Type::Label, name);
}

Command::Type Command::getType() const { 
    return type; 
}

const std::string &Command::getName() const {
    return name; 
}

AddressingMode Command::getAddressingMode() const { 
    return addressingMode; 
}

const std::string &Command::getNumberOrSymbol() const { 
    return numberOrSymbol; 
}

uint32_t Command::getDupNumber() const {
    return dupNumber;
}

int Command::getR1() const { 
    return r1; 
}

int Command::getR2() const { 
    return r2; 
}

int Command::getR3() const { 
    return r3; 
}

Command::Command(Type type, std::string name) : type(type), name(name) {}

Command::Command(Type type, const std::string &name, AddressingMode addressingMode, 
    const std::string &numberOrSymbol, Sign sign, int r1, int r2, int r3)
    : type(type), name(name), addressingMode(addressingMode), 
    numberOrSymbol(numberOrSymbol), sign(sign), r1(r1), r2(r2), r3(r3) {}

static std::string toString(AddressingMode mode) {
    switch (mode) {
    case AddressingMode::None: return "None";
    case AddressingMode::Immediate: return "Immediate";
    case AddressingMode::RegisterDirect: return "RegisterDirect";
    case AddressingMode::MemoryDirect: return "MemoryDirect";
    case AddressingMode::RegisterIndirect: return "RegisterIndirect";
    case AddressingMode::RegisterIndirectWithDisplacement: return "RegisterIndirectWithDisplacement";
    default: return "Unknown";
    }
}

static std::string toString(Command::Type type) {
    switch (type) {
    case Command::Type::Instruction: return "Instruction";
    case Command::Type::Directive: return "Directive";
    case Command::Type::SymbolDefinition: return "SymbolDefinition";
    case Command::Type::Label: return "Label";
    default: return "Unknown";
    }
}

static std::string toString(Command::Sign sign) {
    return (sign == Command::Sign::Plus) ? "+" : "-";
}

std::string Command::toString() const {
    std::ostringstream oss;

    oss << "Command: " << ::toString(type) << ", Name: " << name << "\n";

    if (type == Type::Instruction) {
        oss << "Addressing Mode: " << ::toString(addressingMode) << "\n";
    }

    if (!numberOrSymbol.empty()) {
        oss << "Number/Symbol: " << numberOrSymbol << "\n";
    }

    if (dupNumber) {
        oss << "Number2: " << dupNumber << "\n";
    }

    oss << "Register 1 (r1): " << r1 << "\n";
    oss << "Register 2 (r2): " << r2 << "\n";
    oss << "Register 3 (r3): " << r3 << "\n";

    if (addressingMode == AddressingMode::RegisterIndirectWithDisplacement) {
        oss << "Displacement Sign: " << ::toString(sign) << "\n";
    }

    return oss.str();
}

size_t Command::getMemorySizeWords() const {
    if (type == Type::SymbolDefinition || type == Type::Label) {
        return 0;
    }
    if (type == Type::Directive) {
        if (name == "dup") {
            return dupNumber;
        }
        return name == "dd";
    }
    return
        addressingMode == AddressingMode::None ||
        addressingMode == AddressingMode::RegisterDirect ||
        addressingMode == AddressingMode::RegisterIndirect ? 1 : 0;
}