#include <iostream>
#include <vector>
#include <sstream>
#include <filesystem>
#include <cstdlib>
#include <sys/types.h>
#include <sys/wait.h>

#include "commands.hpp"

int main()
{
  Commands *cmdHandler = new Commands();

  std::string input;
  do
  {
    //  Flush after every std::cout / std:cerr
    std::cout << std::unitbuf;
    std::cerr << std::unitbuf;

    std::cout << "$ ";

    std::getline(std::cin, input);

    cmdHandler->processCommand(input);

  } while (input != "");
}
