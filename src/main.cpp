#include <argparse/argparse.hpp>
#include <iostream>
#include <ncurses.h>
#include <re-vim/buffer.hpp>
#include <re-vim/commands.hpp>
#include <re-vim/globals.hpp>
#include <re-vim/terminal.hpp>
#include <stdexcept>
#include <string>

auto main(int argc, char *argv[]) -> int {
  std::string filename;
  try {
    argparse::ArgumentParser program("re-vim", "0.1.0");

    program.add_argument("filename")
        .help("File to open text editor in.")
        .required();
    program.add_description("TUI text editor, baesd off vim.");

    try {
      program.parse_args(argc, argv);
      filename = program.get<std::string>("filename");
    } catch (const std::runtime_error &e) {
      std::cerr << e.what() << '\n';
      std::cerr << program;
      return 1;
    }
  } catch (const std::exception &e) {
    std::cerr << e.what() << '\n';
    return 1;
  }

  Terminal terminal;

  Buffer buffer(filename);
  buffer.load();
  global_vars::g_buffer = &buffer;

  for (const std::string &line : buffer.getLines()) {
    printw("%s\n", line.data()); // NOLINT(cppcoreguidelines-pro-type-vararg)
  }

  while (!global_vars::f_should_quit) {
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
