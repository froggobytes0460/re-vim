#include <algorithm>
#include <argparse/argparse.hpp>
#include <cctype>
#include <climits>
#include <cstddef>
#include <exception>
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
#include <string_view>

namespace {
/// @brief Escape key as per ASCII.
constexpr char KEY_ESC{27};

/// @brief Usual terminal backspace/deletion key as per ASCII.
constexpr char KEY_TERM_BACKSPACE{127};

/// @brief Tracks in-progress normal-mode input: an accumulated `[count]` and
/// an operator (e.g. `d`) awaiting its motion.
struct NormalState {
  int count = 0;       ///< Accumulated count prefix, 0 means unset.
  char pending_op = 0; ///< Operator awaiting a motion, 0 means none.
};

/// @brief Prints error message on status window with red highligting.
void printError(const std::string_view &err) noexcept {
  wattron(global_vars::status_win, COLOR_PAIR(Terminal::ERROR_PAIR));

  constexpr std::string_view PREFIX{"E: "};
  const int MAX_LEN = std::max(0, getmaxx(global_vars::status_win) -
                                      static_cast<int>(PREFIX.size()) - 1);
  const int LEN = std::min(static_cast<int>(err.size()), MAX_LEN);

  // NOLINTBEGIN(bugprone-suspicious-stringview-data-usage): LEN <=
  // err.size(), bounded read is safe.
  wprintw(global_vars::status_win, "E: %.*s", LEN, err.data());
  // NOLINTEND(bugprone-suspicious-stringview-data-usage)

  wattroff(global_vars::status_win, COLOR_PAIR(Terminal::ERROR_PAIR));
}

/// @brief Parses CLI args.
/// @param[in] argc Argument Count (given by main func)
/// @param[in] argv Argument list (as C-style array given by main func)
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
void drawBuffer(WINDOW *text_win, const Buffer &buffer, int top_line) {
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

/// @brief Deletes up to `count` lines starting at the cursor's line, always
/// leaving at least one line in the buffer.
/// @param[in] count Number of lines requested for deletion.
/// @param[in] cursor The cursor state, indicates start line.
/// @param[in] buffer The buffer to delete lines from.
void deleteLines(int count, Cursor &cursor, Buffer &buffer) noexcept {
  auto remaining = static_cast<int>(buffer.getLines().size());
  int n = std::min(count, std::max(remaining - 1, 0));
  for (int i = 0; i < n; ++i) {
    buffer.deleteLine(cursor.line());
  }
}

/// @brief Handles a keypress while in Normal mode.
/// @param[in] ch The character got from user input.
/// @param[in] cursor The cursor state
/// @param[in] buffer The buffer to navigate.
/// @param[in,out] state Accumulated count/pending-operator state across
/// keypresses.
/// @return False if the command-line was invoked and the caller should
/// `continue` its loop iteration.
auto handleNormalMode(int ch, Cursor &cursor, Buffer &buffer,
                      NormalState &state) noexcept -> bool {
  // Digit accumulation for `[count]` prefix (e.g. `5k`, `12j`). `0` only
  // continues an existing count so it doesn't clash with a future `0` motion
  // (move to start of line).
  if ((ch >= '1' && ch <= '9') || (ch == '0' && state.count != 0)) {
    state.count = (state.count * 10) + (ch - '0');
    return true;
  }

  int count = state.count == 0 ? 1 : state.count;

  switch (ch) {
  case 'a':
    cursor.moveRight(buffer);
    [[fallthrough]];
  case 'i':
    global_vars::mode = Mode::INSERT;
    state = NormalState{};
    break;
  case COMMAND_KEY: {
    state = NormalState{};
    std::string cmd_temp = readCommandLine();
    try {
      std::string_view cmd_str(cmd_temp);
      CmdMatch match = searchForCmd(cmd_str, cursor, buffer);
      if (match.entry == nullptr) {
        break;
      }
      if (match.entry->handler == nullptr) {
        break;
      }
      Command cmd = constructCmdStruct(cmd_str, match);
      match.entry->handler(cmd);
    } catch (std::exception &err) {
      wmove(global_vars::status_win, 0, 0);
      wclrtoeol(global_vars::status_win);
      printError(err.what());
    }
    return false;
  }
  case KEY_RIGHT:
    [[fallthrough]];
  case 'l':
    cursor.moveRight(buffer, count);
    state = NormalState{};
    break;
  case KEY_LEFT:
    [[fallthrough]];
  case 'h':
    cursor.moveLeft(count);
    state = NormalState{};
    break;
  case KEY_UP:
    [[fallthrough]];
  case 'k':
    cursor.moveUp(buffer, count);
    state = NormalState{};
    break;
  case KEY_DOWN:
    [[fallthrough]];
  case 'j':
    cursor.moveDown(buffer, count);
    state = NormalState{};
    break;
  case 'd':
    if (state.pending_op == 'd') {
      deleteLines(count, cursor, buffer);
      state = NormalState{};
    } else {
      state.pending_op = 'd';
    }
    break;
  case KEY_ESC:
    [[fallthrough]];
  default:
    // Unrecognized key aborts any pending operator/count, mirroring vim.
    state = NormalState{};
    break;
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
  switch (ch) {
  case KEY_ESC:
    global_vars::mode = Mode::NORMAL;
    break;
  case KEY_BACKSPACE:
    [[fallthrough]];
  case KEY_TERM_BACKSPACE:
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
    break;
  case KEY_RIGHT:
    curs.moveRight(buf);
    break;
  case KEY_LEFT:
    curs.moveLeft();
    break;
  case KEY_UP:
    curs.moveUp(buf);
    break;
  case KEY_DOWN:
    curs.moveDown(buf);
    break;
  case KEY_ENTER:
    [[fallthrough]];
  case '\n':
    buf.insertLine(curs.line(), curs.col());
    curs.moveDown(buf);
    curs.setCol(0);
    drawBuffer(text_win, buf, top_line);
    break;
  default:
    break;
  }

  if (ch >= 0 && ch <= UCHAR_MAX && isprint(ch) != 0) {
    buf.insertChar(static_cast<char>(ch), curs.line(), curs.col());

    wmove(text_win, curs.line() - top_line, 0);
    wclrtoeol(text_win);
    wprintw(text_win, "%s", buf.getLines().at(curs.line()).c_str());

    curs.moveRightInsert();
  }
}
} // namespace

auto main(int argc, const char *argv[]) -> int {
  std::optional<std::string> filename = parseArgs(argc, argv);
  if (!filename.has_value()) {
    return 1;
  }

  try {
    Terminal terminal;
    Cursor cursor;
    NormalState normal_state;
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
        if (!handleNormalMode(ch, cursor, buffer, normal_state)) {
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
