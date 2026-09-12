#include <filesystem>
#include <fstream>
#include <re-vim/buffer.hpp>
#include <stdexcept>
#include <string>
#include <utility>

namespace fs = std::filesystem;

void Buffer::load() {
  std::ifstream file(filename_);
  for (std::string line; std::getline(file, line);) {
    lines_.push_back(line);
  }
  if (lines_.empty()) {
    lines_.emplace_back();
  }
}

void Buffer::save() {
  std::string tmp_name = filename_ + ".tmp";
  std::ofstream tmp(tmp_name);

  for (const std::string &s : lines_) {
    tmp << s << '\n';
  }
  tmp.close();

  if (tmp.fail()) {
    fs::remove(tmp_name);
    throw std::runtime_error("Failed to save file: " + tmp_name);
  }

  if (fs::exists(filename_)) {
    fs::remove(filename_);
  }
  fs::rename(tmp_name, filename_);

  // Now the buffer isn't modified, since it is in sync with file.
  modified_ = false;
}

void Buffer::insertChar(char c, int line, int col) {
  lines_.at(line).insert(col, 1, c);
  modified_ = true;
}
void Buffer::deleteChar(int line, int col) {
  lines_.at(line).erase(col, 1);
  modified_ = true;
}

void Buffer::insertLine(int line, int col) {
  if (std::cmp_greater_equal(line, lines_.size())) {
    return;
  }
  if (std::cmp_greater(col, lines_.at(line).size())) {
    return;
  }

  if (std::cmp_equal(col, lines_.at(line).size())) {
    lines_.insert(lines_.begin() + line + 1, "");
  } else {
    std::string tail = std::move(lines_.at(line));

    // Since lines_[line] is now in an undefined state, refill it with the part
    // of string before cursor.
    lines_.at(line).assign(tail, 0, col);

    // Remove part of string before cursor from tail now that lines_[line] has
    // recieved it.
    tail.erase(0, col);

    lines_.insert(lines_.begin() + line + 1, std::move(tail));
  }
  modified_ = true;
}

void Buffer::deleteLine(int line) {
  if (line < 0 || std::cmp_greater_equal(line, lines_.size())) {
    return;
  }
  lines_.erase(lines_.begin() + line);
  modified_ = true;
}

void Buffer::joinLineUp(int line) {
  if (line <= 0 || std::cmp_greater_equal(line, lines_.size())) {
    return;
  }
  lines_.at(line - 1) += lines_.at(line);
  deleteLine(line);
}
