#include <ncurses.h>
#include <re-vim/terminal.hpp>

Terminal::Terminal() {
  initscr();
  cbreak();
  noecho();
  curs_set(1);
  set_escdelay(25);

  start_color();
  init_pair(ERROR_PAIR, COLOR_WHITE, COLOR_RED);

  // Cannot initialize these attributes before `initscr()`.
  // NOLINTBEGIN(cppcoreguidelines-prefer-member-initializer)
  text_win_ = newwin(LINES - 1, COLS, 0, 0);
  status_win_ = newwin(1, COLS, LINES - 1, 0);
  // NOLINTEND(cppcoreguidelines-prefer-member-initializer)

  keypad(text_win_, true);
}
Terminal::~Terminal() {
  delwin(text_win_);
  delwin(status_win_);
  endwin();
}
