#include <ncurses.h>
#include <re-vim/commands.hpp>
#include <re-vim/globals.hpp>
#include <string>

constexpr char KEY_ESC{27};
constexpr char KEY_TERM_BACKSPACE{8};
constexpr int HIGHEST_ASCII_SUPPORTED{126};
constexpr int LOWEST_ASCII_SUPPORTED{32};

// NOLINTNEXTLINE(bugprone-exception-escape)
[[nodiscard]] auto readCommandLine() noexcept -> std::string {
  WINDOW *status_win = global_vars::status_win;

  wmove(status_win, 0, 0);
  wclrtoeol(status_win);

  waddch(status_win, COMMAND_KEY);
  wrefresh(status_win);

  std::string cmd{};

  int ch{};
  bool loop_run{true};
  while ((ch = wgetch(status_win)) != '\n' && ch != '\r' && loop_run) {
    switch (ch) {
    case KEY_BACKSPACE:
      [[fallthrough]];
    case KEY_TERM_BACKSPACE: {
      if (!cmd.empty()) {
        cmd.pop_back();
        int y{};
        int x{};

        getyx(status_win, y, x);
        wmove(status_win, y, x - 1);

        wdelch(status_win);
        wrefresh(status_win);
      }
      break;
    }
    case KEY_ESC:
      cmd.clear();
      loop_run = false;
      [[fallthrough]];
    case ERR:
      [[fallthrough]];
    default:
      break;
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
