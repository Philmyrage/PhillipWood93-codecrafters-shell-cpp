#include "commands.hpp"

#include <sstream>
#include <iostream>
#include <filesystem>
#include <cstdlib>
#include <sys/types.h>
#include <sys/wait.h>

const bool Commands::validCommand(const std::string &cmd)
{
    bool valid = false;
    for (int i = 0; i < commands.size(); ++i)
    {
        if (cmd == commands[i])
        {
            return true;
        }
    }
    return valid;
}

void Commands::tokenizeString(std::vector<std::string> &outTokens, const std::string &str)
{
    outTokens.clear();

    std::stringstream inputStream(str);
    std::string temp;
    while (inputStream >> temp)
    {
        outTokens.push_back(temp);
    }
}

const void Commands::echo(const std::vector<std::string> &tokens)
{
    std::string temp = "";
    for (int i = 1; i < tokens.size(); ++i)
    {
        temp += tokens[i] + " ";
    }
    std::cout << temp << std::endl;
}

void Commands::splitString(std::vector<std::string> &outTokens, const std::string &str, const char &delimiter)
{
    std::string temp = "";
    for (int i = 0; i < str.size(); ++i)
    {
        if (str[i] != delimiter)
        {
            temp += str[i];
        }
        if (i == str.size() - 1 || str[i] == delimiter)
        {
            outTokens.push_back(temp);
            temp = "";
        }
    }
}

std::pair<bool, std::string> Commands::searchPath(const std::string &cmd)
{

    std::vector<std::string> pathTokens;
    splitString(pathTokens, std::getenv("PATH"));

    for (int i = 0; i < pathTokens.size(); ++i)
    {
        for (const auto &dir : std::filesystem::directory_iterator(pathTokens[i]))
        {
            std::string t = dir.path().c_str();
            size_t i = t.find_last_of("/");
            t = t.substr(i + 1, t.size());
            if (t == cmd)
            {
                return std::pair(true, dir.path().c_str());
            }
        }
    }
    return std::pair(false, "");
}

void Commands::processCommand(const std::vector<std::string> &tokens)
{
    if (validCommand(tokens[0]))
    {
        if (tokens[0] == "exit")
        {
            exit(std::stoi(tokens[1]));
        }
        else if (tokens[0] == "echo")
        {
            echo(tokens);
        }
        else if (tokens[0] == "type")
        {
            std::pair<bool, std::string> searchedPair = searchPath(tokens[1]);
            if (validCommand(tokens[1]))
            {
                std::cout << tokens[1] << " is a shell builtin" << std::endl;
            }
            else if (searchedPair.first)
            {
                std::cout << tokens[1] << " is " << searchedPair.second << std::endl;
            }
            else
            {
                std::cout << tokens[1] << ": not found" << std::endl;
            }
        }
        else if (tokens[0] == "pwd")
        {
            std::cout << std::filesystem::current_path().string() << std::endl;
        }
    }
    else if (searchPath(tokens[0]).first) // if its a command in the path.
    {
        pid_t pid = fork();
        if (pid == -1)
        {
            std::cerr << "Error, could not create fork." << std::endl;
            return;
        }
        if (pid == 0)
        {
            char *argv[tokens.size()];
            for (int i = 0; i <= tokens.size(); ++i)
            {
                argv[i] = const_cast<char *>(tokens[i].c_str());
            }
            argv[tokens.size()] = nullptr;
            execvp(tokens[0].c_str(), argv);
            std::cerr << "Error: exec failed!" << std::endl;
            kill(pid, 1);
        }
        waitpid(pid, NULL, 0);
    }
    else
    {
        std::cout << tokens[0] << ": command not found" << std::endl;
    }
}
