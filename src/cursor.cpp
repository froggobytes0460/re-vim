#include <algorithm>
#include <re-vim/buffer.hpp>
#include <re-vim/cursor.hpp>

auto Cursor::getCorrectCol(const Buffer &buff) const -> int {
  if (buff.empty()) {
    return 0;
  }
  auto len = static_cast<int>(buff.getLines().at(line_).size());
  return std::min(desired_col_, len);
}

void Cursor::moveRight(const Buffer &buff) noexcept {
  if (buff.empty()) {
    return;
  }
  auto len = static_cast<int>(buff.getLines().at(line_).size());
  if (col_ < len) {
    ++col_;
  }
  desired_col_ = col_;
}

void Cursor::moveLeft() noexcept {
  if (col_ > 0) {
    --col_;
  }
  desired_col_ = col_;
}

void Cursor::moveUp(const Buffer &buff) noexcept {
  if (line_ == 0) {
    return;
  }
  --line_;
  col_ = getCorrectCol(buff);
}

void Cursor::moveDown(const Buffer &buff) noexcept {
  if (line_ + 1 >= static_cast<int>(buff.getLines().size())) {
    return;
  }
  ++line_;
  col_ = getCorrectCol(buff);
}

void Cursor::moveRightInsert() noexcept { desired_col_ = ++col_; }

void Cursor::setCol(int col) noexcept {
  col_ = col;
  desired_col_ = col;
}
