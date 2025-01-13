#pragma once

#include <vector>
#include <string>

class Commands
{
public:
    void processCommand(const std::vector<std::string> &tokens);
    void tokenizeString(std::vector<std::string> &outTokens, const std::string &str);

private:
    std::vector<std::string> commands = {"exit", "echo", "type", "pwd"};

    // Helper Functions
    const bool validCommand(const std::string &cmd);
    void splitString(std::vector<std::string> &outTokens, const std::string &str, const char &delimiter = ':');
    std::pair<bool, std::string> searchPath(const std::string &cmd);

    // Implemented builtin Command Functions..
    const void echo(const std::vector<std::string> &tokens);
};