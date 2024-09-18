#pragma once
#include "Linker.hpp"
#include <unordered_set>
#include <vector>
#include <filesystem>

Linker::Linker(const std::string &rootFilename) : rootFilename(rootFilename) {}

bool Linker::link() {
    return 
        resolveIncludes(rootFilename, commands) && 
        resolveSymbols() && 
        resolveStartDirective() && 
        errors.empty();
}

bool Linker::resolveIncludes(const std::string &filename, std::vector<Command> &collectedCommands) {
    if (hasCircularInclude(filename)) {
        errors.push_back(Error("Circular include detected: " + filename));
        return false;
    }
    if (includedFiles.count(filename) > 0) {
        return true;
    }
    includeStack.push(filename);

    if (!std::filesystem::exists(filename)) {
        errors.push_back(Error("File doesn't exist: " + filename));
        includeStack.pop();
        return false;
    }

    Parser parser(filename);
    if (!parser.parse()) {
        errors.insert(errors.end(), parser.getErrors().begin(), parser.getErrors().end());
        return false;
    }

    includedFiles.insert(filename);
    std::vector<Command> currentFileCommands = parser.getCommands();

    for (const Command &command : currentFileCommands) {
        if (command.getType() == Command::Type::Directive && command.getName() == "include") {
            std::string includeFilename = command.getNumberOrSymbol();

            if (includeFilename.find(".asm") == std::string::npos) {
                includeFilename += ".asm";
            }

            std::filesystem::path rootPath = std::filesystem::path(rootFilename).parent_path();
            std::filesystem::path fullIncludePath = rootPath / includeFilename;

            std::vector<Command> includedFileCommands;
            if (!resolveIncludes(fullIncludePath.string(), includedFileCommands)) {
                return false;
            }

            collectedCommands.insert(collectedCommands.end(), includedFileCommands.begin(), includedFileCommands.end());
        }
        else {
            collectedCommands.push_back(command);
        }
    }


    includeStack.pop();
    return true;
}

bool Linker::hasCircularInclude(const std::string &filename) {
    std::stack<std::string> temp = includeStack;
    while (!temp.empty()) {
        if (temp.top() == filename) {
            return true;
        }
        temp.pop();
    }
    return false;
}

bool Linker::resolveSymbols() {
    // First pass: collect all symbol definitions and labels
    size_t memoryIndex = 0;
    for (const Command &command : commands) {
        if (command.getType() == Command::Type::SymbolDefinition) {
            const std::string &symbol = command.getName();
            if (symbolTable.count(symbol) > 0) {
                errors.push_back(Error("Symbol redefinition: " + symbol));
                return false;
            }
            symbolTable[symbol] = static_cast<uint32_t>(std::stoul(command.getNumberOrSymbol(), nullptr, 16));
        }
        else if (command.getType() == Command::Type::Label) {
            const std::string &symbol = command.getName();
            if (symbolTable.count(symbol) > 0) {
                errors.push_back(Error("Symbol redefinition: " + symbol));
                return false;
            }
            symbolTable[symbol] = memoryIndex;
            memoryIndex += command.getMemorySizeWords();
        }
    }

    // Second pass: resolve all symbol references
    for (const Command &command : commands) {
        if (command.getType() == Command::Type::Instruction || command.getType() == Command::Type::Directive) {
            if (command.getAddressingMode() == AddressingMode::MemoryDirect ||
                command.getAddressingMode() == AddressingMode::RegisterIndirectWithDisplacement) {
                const std::string &symbol = command.getNumberOrSymbol();
                if (symbolTable.count(symbol) == 0) {
                    errors.push_back(Error("Undefined symbol: " + symbol));
                    return false;
                }
            }
        }
    }

    return true;
}

bool Linker::resolveStartDirective() {
    for (const Command &command : commands) {
        if (command.getType() == Command::Type::Directive && command.getName() == "start") {
            if (startFound) {
                errors.push_back(Error("Multiple start directives found."));
                return false;
            }
            startFound = true;
        }
    }

    if (!startFound) {
        errors.push_back(Error("No start directive found."));
        return false;
    }

    return true;
}

const std::vector<Command> &Linker::getCommands() const {
    return commands;
}

const std::vector<Error> &Linker::getErrors() const {
    return errors;
}

bool Linker::hasErrors() const {
    return !errors.empty();
}
