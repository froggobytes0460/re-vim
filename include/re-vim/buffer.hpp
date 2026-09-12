#pragma once

#include <string>
#include <utility>
#include <vector>

/// @brief Handles buffer for each file. Uses line array datastructure to hanlde
/// file I/O.
class Buffer {
private:
  std::vector<std::string> lines_; ///< Vector of lines, seperated by '\\n'
  std::string filename_;           ///< Name of file
  bool modified_{false};           ///< True if any edits been made.
public:
  explicit Buffer(std::string filename) : filename_(std::move(filename)) {}

  /// @brief True if the file is empty.
  [[nodiscard]] auto empty() const -> bool { return lines_.empty(); };

  /// @brief Loads file from `filename_`. Guarantees at least one (possibly
  /// empty) line if the file cannot be opened or is empty.
  void load();

  /// @brief Saves file to `filename_`. The replacement of file is atomic (on
  /// most filesystems atleast).
  void save();

  /// @brief Getter method for `lines_`.
  [[nodiscard]] auto getLines() const -> const std::vector<std::string> & {
    return lines_;
  }

  /// @brief State of syncronization between buffer and file.
  [[nodiscard]] auto isSyncronized() const -> bool { return !modified_; }

  /// @brief Insertion of character in between lines.
  /// @param[in] c The character to insert.
  /// @param[in] line The line index.
  /// @param[in] col the index of line where to add char.
  void insertChar(char c, int line, int col);

  /// @brief Deletion of character in between lines.
  /// @param[in] line The line index.
  /// @param[in] col the index of line where to delete char.
  void deleteChar(int line, int col);

  /// @brief Insertion of character in between lines.
  /// @param[in] line The line index.
  /// @param[in] col The column index of the text where to split line.
  void insertLine(int line, int col);

  /// @brief Deletion of line.
  /// @param[in] line The line number to delete.
  void deleteLine(int line);

  /// @brief Joins `line` into `line - 1` (appends its content), then deletes
  /// `line`.
  /// @note No-op if `line` is the first line (nothing to join into).
  /// @param[in] line The line to join upward into its predecessor.
  void joinLineUp(int line);
};
