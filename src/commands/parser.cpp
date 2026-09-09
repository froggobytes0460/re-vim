#include <ncurses.h>
#include <re-vim/commands.hpp>
#include <re-vim/globals.hpp>
#include <string>

constexpr char KEY_ESC{27};
constexpr char KEY_TERM_BACKSPACE{8};
constexpr int HIGHEST_ASCII_SUPPORTED{126};
constexpr int LOWEST_ASCII_SUPPORTED{32};

[[nodiscard]] auto readCommandLine() noexcept -> std::string {
  WINDOW *status_win = global_vars::status_win;

  wmove(status_win, 0, 0);
  wclrtoeol(status_win);

  waddch(status_win, COMMAND_KEY);
  wrefresh(status_win);

  std::string cmd{};

  int ch{};
  while ((ch = wgetch(status_win)) != '\n' && ch != '\r') {
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

        getyx(status_win, y, x);
        wmove(status_win, y, x - 1);

        wdelch(status_win);
        wrefresh(status_win);
      }
      continue;
    }

    if (ch < LOWEST_ASCII_SUPPORTED || ch > HIGHEST_ASCII_SUPPORTED) {
      continue;
    }

    waddch(status_win, ch);
    cmd += static_cast<char>(ch);
    wrefresh(status_win);
  }

  wmove(status_win, 0, 0);
  wclrtoeol(status_win);
  wrefresh(status_win);

  return cmd;
}
