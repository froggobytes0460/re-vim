#include <algorithm>
#include <argparse/argparse.hpp>
#include <cctype>
#include <climits>
#include <iostream>
#include <ncurses.h>
#include <optional>
#include <re-vim/buffer.hpp>
#include <re-vim/commands.hpp>
#include <re-vim/cursor.hpp>
#include <re-vim/globals.hpp>
#include <re-vim/modes.hpp>
#include <re-vim/terminal.hpp>
#include <stdexcept>
#include <string>

/// @brief Escape key as per ASCII.
constexpr char KEY_ESC{27};

/// @brief Usual terminal backspace/deletion key as per ASCII.
constexpr char KEY_TERM_BACKSPACE{127};

/// @brief Parses CLI args.
/// @param[in] argc Argument Count (given by main func)
/// @param[in] argv Argument list (as C-style array given by main func)
/// @param[out] ok Set to false if parsing failed.
/// @return Filename argument, or `nullopt` + prints usage/error on failure.
auto parseArgs(int argc, const char **argv) noexcept
    -> std::optional<std::string> {
  try {
    argparse::ArgumentParser program("re-vim", "0.1.0");
    program.add_argument("filename")
        .help("File to open text editor in.")
        .required();
    program.add_description("TUI text editor, baesd off vim.");

    try {
      program.parse_args(argc, argv);
    } catch (const std::runtime_error &e) {
      std::cerr << e.what() << '\n';
      std::cerr << program;
      return std::nullopt;
    }

    return program.get<std::string>("filename");
  } catch (const std::exception &e) {
    std::cerr << e.what() << '\n';
    return std::nullopt;
  }
}

/// @brief Prints the buffer lines visible in the given viewport.
/// @param[in] text_win Window to draw into.
/// @param[in] buffer Buffer to draw from.
/// @param[in] top_line First buffer line visible at the top of `text_win`.
void drawBuffer(WINDOW *text_win, const Buffer &buffer, int top_line) noexcept {
  werase(text_win);
  wmove(text_win, 0, 0);
  const auto &lines = buffer.getLines();
  int height = getmaxy(text_win);
  int last = std::min(static_cast<int>(lines.size()), top_line + height);
  for (int i = top_line; i < last; ++i) {
    wprintw(text_win, "%s\n", lines.at(i).c_str());
  }
}

/// @brief Draws the mode indicator on the status window.
/// @param[in] status_win Window where status messages and command input exist.
void drawStatusLine(WINDOW *status_win) noexcept {
  wmove(status_win, 0, 0);
  wclrtoeol(status_win);
  switch (global_vars::mode) {
  case Mode::NORMAL:
    wprintw(status_win, "[NORMAL]");
    break;
  case Mode::INSERT:
    wprintw(status_win, "[INSERT]");
    break;
  }
}

/// @brief Clamps `top_line` so `cursor` stays within the visible viewport.
/// @param[in] cursor Cursor whose line must remain visible.
/// @param[in,out] top_line First buffer line visible at the top of the
/// window.
/// @param[in] height Number of visible rows in the text window.
void clampViewport(const Cursor &cursor, int &top_line, int height) noexcept {
  if (cursor.line() < top_line) {
    top_line = cursor.line();
  } else if (cursor.line() >= top_line + height) {
    top_line = cursor.line() - height + 1;
  }
}

/// @brief Handles a keypress while in Normal mode.
/// @param[in] ch The character got from user input.
/// @param[in] cursor The cursor state
/// @param[in] buffer The buffer to navigate.
/// @return False if the command-line was invoked and the caller should
/// `continue` its loop iteration.
auto handleNormalMode(int ch, Cursor &cursor, Buffer &buffer) noexcept -> bool {
  if (ch == 'i') {
    global_vars::mode = Mode::INSERT;
    return true;
  }
  if (ch == 'a') {
    cursor.moveRight(buffer);
    global_vars::mode = Mode::INSERT;
    return true;
  }
  if (ch == COMMAND_KEY) {
    std::string cmd = readCommandLine();
    callCmd(cmd);
    return false;
  }

  if (ch == KEY_RIGHT || ch == 'l') {
    cursor.moveRight(buffer);
  } else if (ch == KEY_LEFT || ch == 'h') {
    cursor.moveLeft();
  } else if (ch == KEY_UP || ch == 'k') {
    cursor.moveUp(buffer);
  } else if (ch == KEY_DOWN || ch == 'j') {
    cursor.moveDown(buffer);
  }
  return true;
}

/// @brief Handles a keypress while in Insert mode.
/// @param[in] ch The character got from user input.
/// @param[in] buf The buffer to edit.
/// @param[in] curs The cursor state.
/// @param[in] text_win Window containing text.
/// @param[in] top_line The topmost line in the buffer currently, used for
/// proper scrolling.
void handleInsertMode(int ch, Buffer &buf, Cursor &curs, WINDOW *text_win,
                      int top_line) {
  if (ch == KEY_ESC) {
    global_vars::mode = Mode::NORMAL;
  } else if (ch == KEY_BACKSPACE || ch == KEY_TERM_BACKSPACE) {
    if (curs.col() > 0) {
      buf.deleteChar(curs.line(), curs.col() - 1);

      wmove(text_win, curs.line() - top_line, 0);
      wclrtoeol(text_win);
      wprintw(text_win, "%s", buf.getLines().at(curs.line()).c_str());

      curs.moveLeft();
    } else if (curs.line() > 0) {
      int prev_len =
          static_cast<int>(buf.getLines().at(curs.line() - 1).size());
      buf.joinLineUp(curs.line());
      curs.moveUp(buf);
      curs.setCol(prev_len);
      drawBuffer(text_win, buf, top_line);
    }
  } else if (ch == KEY_RIGHT) {
    curs.moveRight(buf);
  } else if (ch == KEY_LEFT) {
    curs.moveLeft();
  } else if (ch == KEY_UP) {
    curs.moveUp(buf);
  } else if (ch == KEY_DOWN) {
    curs.moveDown(buf);
  } else if (ch == KEY_ENTER || ch == '\n' || ch == '\r') {
    buf.insertLine(curs.line(), curs.col());
    curs.moveDown(buf);
    curs.setCol(0);
    drawBuffer(text_win, buf, top_line);
  } else if (ch >= 0 && ch <= UCHAR_MAX && isprint(ch) != 0) {
    buf.insertChar(static_cast<char>(ch), curs.line(), curs.col());

    wmove(text_win, curs.line() - top_line, 0);
    wclrtoeol(text_win);
    wprintw(text_win, "%s", buf.getLines().at(curs.line()).c_str());

    curs.moveRightInsert();
  }
}

auto main(int argc, const char *argv[]) noexcept -> int {
  std::optional<std::string> filename = parseArgs(argc, argv);
  if (!filename.has_value()) {
    return 1;
  }

  try {
    Terminal terminal;
    Cursor cursor;
    WINDOW *text_win = terminal.textWin();
    WINDOW *status_win = terminal.statusWin();
    global_vars::status_win = status_win;

    Buffer buffer(*filename);
    buffer.load();
    global_vars::curr_buffer = &buffer;

    int top_line = 0;
    int viewport_height = getmaxy(text_win);

    drawBuffer(text_win, buffer, top_line);
    move(0, 0);

    while (!global_vars::termination_flag) {
      int ch = wgetch(text_win);
      if (ch == ERR) {
        continue;
      }

      switch (global_vars::mode) {
      case Mode::NORMAL:
        if (!handleNormalMode(ch, cursor, buffer)) {
          continue;
        }
        break;
      case Mode::INSERT:
        handleInsertMode(ch, buffer, cursor, text_win, top_line);
        break;
      }

      int prev_top_line = top_line;
      clampViewport(cursor, top_line, viewport_height);
      if (top_line != prev_top_line) {
        drawBuffer(text_win, buffer, top_line);
      }

      drawStatusLine(status_win);

      wmove(text_win, cursor.line() - top_line, cursor.col());
      wnoutrefresh(status_win);
      wnoutrefresh(text_win);
      doupdate();
    }
  } catch (const std::exception &e) {
    std::cerr << e.what() << '\n';
    return 1;
  }
  return 0;
}
