#include <filesystem>
#include <fstream>
#include <re-vim/buffer.hpp>
#include <stdexcept>
#include <string>

namespace fs = std::filesystem;

void Buffer::load() {
  std::ifstream file(filename_);
  for (std::string line; std::getline(file, line);) {
    lines_.push_back(line);
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
}
