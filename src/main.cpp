#include <ncurses.h>
#include <re-vim/commands.hpp>
#include <re-vim/terminal.hpp>
#include <string>

auto main() -> int {
  Terminal terminal;

  int ch{};

  while (!global_flag::f_should_quit && (ch = getch()) != 0) {
    if (ch == ERR) {
      continue;
    }
    if (ch == COMMAND_KEY) {
      std::string cmd = readCommandLine();
      callCmd(cmd);
      continue;
    }
    printw("You pressed: %c\n", // NOLINT(cppcoreguidelines-pro-type-vararg)
           ch);
  }
  return 0;
}
