#include "Token.hpp"
#include "Parser.hpp"
#include "Linker.hpp"
#include <iostream>
#include <filesystem>
#include <algorithm>
#include <sstream>

static const std::string directoryPath = "C:/assembly/";

void displayError(const std::string &message) {
    std::cout << "Error: " << message << std::endl;
    std::cout << "Available commands: " << std::endl;
    std::cout << "  exit              - Exit the program" << std::endl;
    std::cout << "  list              - List all files in the directory" << std::endl;
    std::cout << "  tokenize <file>   - Tokenize a file with .asm extension" << std::endl;
    std::cout << "  parse <file>      - Parse a file with .asm extension" << std::endl;
}

void handleListCommand() {
    try {
        std::cout << "Files in directory '" << directoryPath << "':" << std::endl;
        for (const auto &entry : std::filesystem::directory_iterator(directoryPath)) {
            if (entry.is_regular_file()) {
                std::cout << "  " << entry.path().filename().string() << std::endl;
            }
        }
    }
    catch (const std::filesystem::filesystem_error &e) {
        displayError("Could not access the directory: " + std::string(e.what()));
    }
}

void handleTokenizeCommand(const std::string &filename) {
    std::string fullFilePath = directoryPath + filename + ".asm";

    Tokenizer tokenizer(fullFilePath);

    if (tokenizer.tokenize()) {
        std::cout << "Successfully tokenized: " << filename << ".asm" << std::endl;
        std::cout << "Tokens:" << std::endl;
        for (const auto &token : tokenizer.getTokens()) {
            std::string typeStr;
            switch (token.type) {
            case Token::Type::Name: typeStr = "Name  "; break;
            case Token::Type::Number: typeStr = "Number"; break;
            case Token::Type::Symbol: typeStr = "Symbol"; break;
            default: typeStr = "Unknown"; break;
            }
            std::cout << "  [" << typeStr << "] " << token.value << std::endl;
        }
    }
    else {
        std::cout << "Errors occurred during tokenization:" << std::endl;
        for (const auto &error : tokenizer.getErrors()) {
            std::cout << error.getMessage() << std::endl;
        }
    }
}

// New function to handle the 'parse' command
void handleParseCommand(const std::string &filename) {
    std::string fullFilePath = directoryPath + filename + ".asm";

    Parser parser(fullFilePath);

    if (parser.parse()) {
        std::cout << "Successfully parsed: " << filename << ".asm" << std::endl;
        std::cout << "Commands:" << std::endl;
        for (const auto &command : parser.getCommands()) {
            std::cout << command.toString() << std::endl;
        }
    }
    else {
        std::cout << "Errors occurred during parsing:" << std::endl;
        for (const auto &error : parser.getErrors()) {
            std::cout << error.getMessage() << std::endl;
        }
    }
}
void handleLinkCommand(const std::string &filename) {
    std::string fullFilePath = directoryPath + filename + ".asm";

    Linker linker(fullFilePath);

    if (linker.link()) {
        std::cout << "Successfully linked: " << filename << ".asm" << std::endl;
        std::cout << "Linked commands:" << std::endl;
        for (const auto &command : linker.getCommands()) {
            std::cout << command.toString() << std::endl;
        }
    }
    else {
        std::cout << "Errors occurred during linking:" << std::endl;
        for (const auto &error : linker.getErrors()) {
            std::cout << error.getMessage() << std::endl;
        }
    }
}

void processCommand(const std::string &input) {
    std::istringstream iss(input);
    std::string command;
    iss >> command;

    // Convert command to lowercase for case-insensitive comparison
    std::transform(command.begin(), command.end(), command.begin(), ::tolower);

    if (command == "exit") {
        std::cout << "Exiting the program." << std::endl;
        exit(0);
    }
    else if (command == "list") {
        handleListCommand();
    }
    else if (command == "tokenize") {
        std::string filename;
        iss >> filename;
        if (filename.empty()) {
            displayError("You must specify a filename to tokenize. Usage: tokenize <file>");
        }
        else {
            handleTokenizeCommand(filename);
        }
    }
    else if (command == "parse") {
        std::string filename;
        iss >> filename;
        if (filename.empty()) {
            displayError("You must specify a filename to parse. Usage: parse <file>");
        }
        else {
            handleParseCommand(filename);
        }
    }
    else if (command == "link") {
        std::string filename;
        iss >> filename;
        if (filename.empty()) {
            displayError("You must specify a filename to link. Usage: link <file>");
        }
        else {
            handleLinkCommand(filename);
        }
    }
    else {
        displayError("Unknown command: " + command);
    }
}

int main() {
    std::string input;

    std::cout << "Welcome to the Assembly Tokenizer and Parser CLI!" << std::endl;
    std::cout << "Type 'exit' to quit, 'list' to list files, 'tokenize <filename>' to tokenize an assembly file, or 'parse <filename>' to parse a file." << std::endl;

    while (true) {
        std::cout << "$ ";  // Prompt for user input
        std::getline(std::cin, input);

        if (!input.empty()) {
            processCommand(input);
        }
    }

    return 0;
}
