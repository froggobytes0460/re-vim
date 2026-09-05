#pragma once

/// @brief  RAII class wrapper on `ncurses` init/destruct. Deletes all copy and
/// move constructors/functions, as this may cause undefined behaviour if
/// misused.
class Terminal {
public:
  Terminal();
  ~Terminal();

  Terminal(Terminal &&) = delete;
  auto operator=(Terminal &&) -> Terminal & = delete;
  Terminal(const Terminal &) = delete;
  auto operator=(const Terminal &) -> Terminal & = delete;
};
