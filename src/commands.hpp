#pragma once

#include <vector>
#include <string>

class Commands
{
public:
    void processCommand(const std::string &str);

private:
    std::vector<std::string> commands = {"exit", "echo", "type", "pwd", "cd"};
    std::string quote = "";

    // Helper Functions
    const bool validCommand(const std::string &cmd);
    void tokenizeString(std::vector<std::string> &outTokens, const std::string &str);
    void splitString(std::vector<std::string> &outTokens, const std::string &str, const char &delimiter = ':');
    std::pair<bool, std::string> searchPath(const std::string &cmd);

    // Implemented builtin Command Functions..
    const void echo(const std::vector<std::string> &tokens);
    void changeDirectory(const std::vector<std::string> &inTokens);
};