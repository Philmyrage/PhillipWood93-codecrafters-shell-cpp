#include "commands.hpp"

#include <sstream>
#include <iostream>
#include <filesystem>
#include <cstdlib>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <regex>

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

    std::string temp;
    // get the commands not in quotes...
    std::stringstream inputStream(str);
    while (inputStream >> temp)
    {
        if (temp.find('\'') == std::string::npos)
        {
            outTokens.push_back(temp);
        }
    }
    // if the first token is echo do some special hacky stuff...
    if (outTokens.size() == 1 && outTokens[0] == "echo")
    {
        // construct the echo token string
        std::string tok = str.substr(6, str.size() - 7);
        std::string t;
        for (int i = 0; i < tok.size(); ++i)
        {
            if (tok[i] != '\'')
            {
                t += tok[i];
            }
        }
        outTokens.push_back(t);

        // return so we dont create more tokens later in the function.
        return;
    }

    // get the tokens inside quotes.
    std::regex pattern("'([^']*)'");
    std::sregex_token_iterator iter(str.begin(), str.end(), pattern, 1);
    std::sregex_token_iterator end;
    for (; iter != end; ++iter)
    {
        outTokens.push_back(*iter);
    }
}

// refactor to take a string and tokenize if required.
const void Commands::echo(const std::vector<std::string> &tokens)
{
    std::string temp = "";
    for (int i = 1; i < tokens.size(); ++i)
    {
        temp += tokens[i] + " ";
    }
    std::cout << temp << std::endl;
}

void Commands::changeDirectory(const std::vector<std::string> &inTokens)
{
    if (std::filesystem::exists(inTokens[1]))
    {
        std::filesystem::current_path(inTokens[1]);
    }
    else if (inTokens[1] == "~")
    {
        std::filesystem::current_path(std::getenv("HOME"));
    }
    else
    {
        std::cerr << "cd: " << inTokens[1] << ": No such file or directory" << std::endl;
    }
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
        if (std::filesystem::exists(pathTokens[i]))
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
        else
        {
            continue;
        }
    }
    return std::pair(false, "");
}

void Commands::processCommand(const std::string &str)
{
    std::vector<std::string> tokens;
    tokenizeString(tokens, str);

    // A debug output to test tokens....
    //  for (int i = 0; i < tokens.size(); ++i)
    //  {
    //      std::cout << tokens[i] + ": 1" << std::endl;
    //  }

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
        else if (tokens[0] == "cd")
        {
            changeDirectory(tokens);
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
            std::string path = searchPath(tokens[0]).second;
            char *argv[tokens.size() + 1];
            for (int i = 0; i < tokens.size(); ++i)
            {
                argv[i] = const_cast<char *>(tokens[i].c_str());
            }
            argv[tokens.size()] = nullptr;

            int fd = open("/dev/null", O_RDWR);
            dup2(fd, STDERR_FILENO);
            close(fd);

            execv(path.c_str(), argv);
            std::cerr << "Error: exec failed!" << std::endl;
            kill(pid, 1);
        }
        else
        {
            waitpid(pid, NULL, 0);
        }
    }
    else
    {
        std::cout << tokens[0] << ": command not found" << std::endl;
    }
}
