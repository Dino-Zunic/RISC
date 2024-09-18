#pragma once

#include "Token.hpp"
#include <unordered_set>

enum class AddressingMode {
    None,
    Immediate,
    RegisterDirect,
    MemoryDirect,
    RegisterIndirect,
    RegisterIndirectWithDisplacement
};

class Command {
public:
    enum class Type {
        Instruction,
        Directive,
        SymbolDefinition,
        Label
    };
    enum class Sign {
        Plus,
        Minus
    };
    static Command createInstruction0(const std::string &name);
    static Command createInstruction(const std::string &name, AddressingMode addressingMode, const std::string &numberOrSymbol, Sign sign, int r1, int r2 = 0);
    static Command createInstruction3(const std::string &name, int r1, int r2, int r3);
    static Command createDirective(const std::string &name, const std::string &numberOrSymbol, uint32_t dupNumber = 0);
    static Command createSymbolDefinition(const std::string &name, const std::string &number);
    static Command createLabel(const std::string &name);
    Type getType() const;
    const std::string &getName() const;
    AddressingMode getAddressingMode() const;
    const std::string &getNumberOrSymbol() const;
    uint32_t getDupNumber() const;
    int getR1() const;
    int getR2() const;
    int getR3() const;
    std::string toString() const;
    uint32_t getMemorySizeWords() const;

private:
    Type type;
    AddressingMode addressingMode = AddressingMode::None;
    std::string name;
    std::string numberOrSymbol;
    uint32_t dupNumber = 0; // only used for dup
    Sign sign; // used for register indirect with displacement
    int r1 = 0;
    int r2 = 0;
    int r3 = 0;

    Command(Type type, std::string name);

    Command(Type type, const std::string &name, AddressingMode addressingMode, const std::string &numberOrSymbol, 
        Sign sign, int r1 = 0, int r2 = 0, int r3 = 0);
};