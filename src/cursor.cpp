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

void Cursor::moveRight(const Buffer &buff, int count) {
  if (buff.empty()) {
    return;
  }
  auto len = static_cast<int>(buff.getLines().at(line_).size());
  col_ = std::min(col_ + count, len);
  desired_col_ = col_;
}

void Cursor::moveLeft(int count) noexcept {
  col_ = std::max(col_ - count, 0);
  desired_col_ = col_;
}

void Cursor::moveUp(const Buffer &buff, int count) {
  line_ = std::max(line_ - count, 0);
  col_ = getCorrectCol(buff);
}

void Cursor::moveDown(const Buffer &buff, int count) {
  auto last = static_cast<int>(buff.getLines().size()) - 1;
  line_ = std::min(line_ + count, std::max(last, 0));
  col_ = getCorrectCol(buff);
}

void Cursor::moveRightInsert() noexcept { desired_col_ = ++col_; }

void Cursor::setCol(int col) noexcept {
  col_ = col;
  desired_col_ = col;
}
