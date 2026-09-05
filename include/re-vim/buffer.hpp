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
public:
  bool modified{false}; ///< True if any edits been made.
  explicit Buffer(std::string filename) : filename_(std::move(filename)) {}

  /// @brief True if the file is empty.
  [[nodiscard]] auto empty() const -> bool { return lines_.empty(); };

  /// @brief Loads file from `filename_`, empty buffer if cannot be opened.
  void load();

  /// @brief Saves file to `filename_`. The replacement of file is atomic (on
  /// most filesystems atleast).
  void save();

  /// @brief Getter method for `lines_`.
  [[nodiscard]] auto getLines() const -> const std::vector<std::string> & {
    return lines_;
  }
};
