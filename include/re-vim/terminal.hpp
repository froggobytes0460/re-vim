#pragma once

#include <ncurses.h>

/// @brief  RAII class wrapper on `ncurses` init/destruct. Deletes all copy and
/// move constructors/functions, as this may cause undefined behaviour if
/// misused. Owns the two windows the editor draws into: a text window for
/// buffer content and a 1-row status window pinned to the bottom, so the two
/// regions can never overwrite each other.
class Terminal {
private:
  WINDOW *text_win_{};
  WINDOW *status_win_{};

public:
  Terminal();
  ~Terminal();

  Terminal(Terminal &&) = delete;
  auto operator=(Terminal &&) -> Terminal & = delete;
  Terminal(const Terminal &) = delete;
  auto operator=(const Terminal &) -> Terminal & = delete;

  /// @brief Getter method for the buffer/text window.
  [[nodiscard]] auto textWin() const -> WINDOW * { return text_win_; }

  /// @brief Getter method for the status line window.
  [[nodiscard]] auto statusWin() const -> WINDOW * { return status_win_; }
};
