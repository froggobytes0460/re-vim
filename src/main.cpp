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
auto parseArgs(int argc, char **argv) -> std::optional<std::string> {
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

/// @brief Prints the buffer contents from the top of the screen.
void drawBuffer(const Buffer &buffer) {
  move(0, 0);
  for (const std::string &line : buffer.getLines()) {
    printw("%s\n", line.c_str());
  }
}

/// @brief Draws the mode indicator on the last screen line.
void drawStatusLine() {
  move(LINES - 1, 0);
  clrtoeol();
  switch (global_vars::mode) {
  case Mode::NORMAL:
    printw("[NORMAL]");
    break;
  case Mode::INSERT:
    printw("[INSERT]");
    break;
  }
}

/// @brief Handles a keypress while in Normal mode.
/// @return False if the command-line was invoked and the caller should
/// `continue` its loop iteration.
auto handleNormalMode(int ch, Cursor &cursor, Buffer &buffer) -> bool {
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
void handleInsertMode(int ch, Buffer &buf, Cursor &curs) {
  if (ch == KEY_ESC) {
    global_vars::mode = Mode::NORMAL;
  } else if (ch == KEY_BACKSPACE || ch == KEY_TERM_BACKSPACE) {
    if (curs.col() > 0) {
      buf.deleteChar(curs.line(), curs.col() - 1);

      move(curs.line(), 0);
      clrtoeol();
      printw("%s", buf.getLines().at(curs.line()).c_str());

      curs.moveLeft();
    } else if (curs.line() > 0) {
      int prev_len =
          static_cast<int>(buf.getLines().at(curs.line() - 1).size());
      buf.joinLineUp(curs.line());
      curs.moveUp(buf);
      curs.setCol(prev_len);
      drawBuffer(buf);
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
    drawBuffer(buf);
  } else if (ch >= 0 && ch <= UCHAR_MAX && isprint(ch) != 0) {
    buf.insertChar(static_cast<char>(ch), curs.line(), curs.col());

    move(curs.line(), 0);
    clrtoeol();
    printw("%s", buf.getLines().at(curs.line()).c_str());

    curs.moveRightInsert();
  }
}

auto main(int argc, char *argv[]) -> int {
  std::optional<std::string> filename = parseArgs(argc, argv);
  if (!filename.has_value()) {
    return 1;
  }

  try {
    Terminal terminal;
    Cursor cursor;

    Buffer buffer(*filename);
    buffer.load();
    global_vars::g_buffer = &buffer;

    drawBuffer(buffer);

    while (!global_vars::termination_flag) {
      int ch = getch();
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
        handleInsertMode(ch, buffer, cursor);
        break;
      }

      drawStatusLine();

      move(cursor.line(), cursor.col());
      refresh();
    }
  } catch (const std::exception &e) {
    std::cerr << e.what() << '\n';
    return 1;
  }
  return 0;
}
