#include <ncurses.h>
#include <re-vim/terminal.hpp>

Terminal::Terminal() {
  initscr();
  cbreak();
  noecho();
  keypad(stdscr, true);
  curs_set(1);
  set_escdelay(25);
}
Terminal::~Terminal() { endwin(); }
