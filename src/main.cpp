#include <argparse/argparse.hpp>
#include <exception>
#include <iostream>
#include <ncurses.h>
#include <re-vim/commands.hpp>
#include <re-vim/terminal.hpp>
#include <string>

auto main(int argc, char *argv[]) -> int {
  argparse::ArgumentParser program("re-vim", "0.1.0");

  program.add_argument("filename")
      .help("File to open text editor in.")
      .required();

  try {
    program.parse_args(argc, argv);
  } catch (const std::exception &e) {
    std::cerr << e.what() << '\n';
    std::cerr << program;
    return 1;
  }

  Terminal terminal;

  while (!global_flag::f_should_quit) {
    int ch = getch();
    if (ch == ERR) {
      continue;
    }
    if (ch == COMMAND_KEY) {
      std::string cmd = readCommandLine();
      callCmd(cmd);
      continue;
    }
  }
  return 0;
}
