#pragma once
#include <string>

class Error {
public:
    static constexpr int NO_LINE = -1;

    Error(const std::string &message, int line = NO_LINE, const std::string &filename = "");

    std::string getMessage() const;

private:
    std::string message;
    int line;
    std::string filename;
};
