#pragma once

// @brief RAII on handling `ncurses` termminal window initiliazation and
// destruction.
class Terminal {
public:
  Terminal();
  ~Terminal();

  Terminal(Terminal &&) = delete;
  auto operator=(Terminal &&) -> Terminal & = delete;
  Terminal(const Terminal &) = delete;
  auto operator=(const Terminal &) -> Terminal & = delete;
};
