#include "Error.hpp"

Error::Error(const std::string &message, int line, const std::string &filename)
    : message(message), line(line), filename(filename) {}

std::string Error::getMessage() const {
    std::string result = "Error";
    if (line != NO_LINE) { 
        result += " on line " + std::to_string(line);
    }
    
    if (!filename.empty()) {
        result += " in file " + filename;
    }
    result += ": " + message;
    return result;
}