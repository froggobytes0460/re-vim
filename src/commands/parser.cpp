#include <ncurses.h>
#include <re-vim/commands.hpp>
#include <string>

constexpr char KEY_ESC{27};
constexpr char KEY_TERM_BACKSPACE{8};
constexpr int HIGHEST_ASCII_SUPPORTED{126};
constexpr int LOWEST_ASCII_SUPPORTED{32};

[[nodiscard("Do not ignore command line input from user.")]] auto
readCommandLine() -> std::string {
  int prev_x{};
  int prev_y{};
  getyx(stdscr, prev_y, prev_x);

  move(LINES - 1, 0);
  clrtoeol();

  addch(COMMAND_KEY);
  refresh();

  std::string cmd{};

  int ch{};
  while ((ch = getch()) != '\n' && ch != '\r') {
    if (ch == ERR) {
      continue;
    }

    if (ch == KEY_ESC) {
      cmd.clear();
      break;
    }

    if (ch == KEY_BACKSPACE || ch == KEY_TERM_BACKSPACE) {
      if (!cmd.empty()) {
        cmd.pop_back();
        int y{};
        int x{};

        getyx(stdscr, y, x);
        move(y, x - 1);

        delch();
        refresh();
      }
      continue;
    }

    if (ch < LOWEST_ASCII_SUPPORTED || ch > HIGHEST_ASCII_SUPPORTED) {
      continue;
    }

    addch(ch);
    cmd += static_cast<char>(ch);
    refresh();
  }

  move(LINES - 1, 0);
  clrtoeol();

  move(prev_y, prev_x);

  return cmd;
}
