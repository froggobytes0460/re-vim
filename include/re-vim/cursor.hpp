#pragma once

#include <re-vim/buffer.hpp>

/// @brief Manages cursor state. This also supports `curswant` behavior.
class Cursor {
private:
  int line_{};        ///< Line index of cursor.
  int col_{};         ///< Column index of cursor.
  int desired_col_{}; ///<  The column index to go if possible.

  /// @brief Helper method to get which column to go, with buffer as added
  /// context.
  /// @param[in] buff Uses buffer to get if the cursor can go to `desired_col_`.
  [[nodiscard]] auto getCorrectCol(const Buffer &buff) const -> int;

public:
  /// @brief Getter method for `line_`.
  [[nodiscard]] auto line() const -> int { return line_; }

  /// @brief Getter method for `col_`.
  [[nodiscard]] auto col() const -> int { return col_; }

  // Move management.

  /// @brief Moves cursor right, stays on EOL if reached.
  /// @param[in] buff Uses buffer to safeguard move ops.
  void moveRight(const Buffer &buff) noexcept;

  /// @brief Moves cursor left, stays on first char if reached.
  void moveLeft() noexcept;

  /// @brief Moves cursor up, stays on first line if reached.
  /// @param[in] buff Uses buffer to safeguard move ops.
  void moveUp(const Buffer &buff) noexcept;

  /// @brief Moves cursor down, stays on EOF if reached.
  /// @param[in] buff Uses buffer to safeguard move ops.
  void moveDown(const Buffer &buff) noexcept;

  /// @brief Advance one column after insertion.
  void moveRightInsert() noexcept;

  /// @brief Directly sets the column, e.g. to reposition after a line split
  /// or join. Also updates `desired_col_`.
  /// @param[in] col Column to set.
  void setCol(int col) noexcept;
};
