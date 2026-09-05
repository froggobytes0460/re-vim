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
  std::string tmp = filename_ + ".tmp";
  std::ofstream tmp_file(tmp);

  for (const std::string &s : lines_) {
    tmp_file << s << '\n';
  }
  tmp_file.close();

  if (tmp_file.fail()) {
    fs::remove(tmp);
    throw std::runtime_error("Failed to save file: " + tmp);
  }

  if (fs::exists(filename_)) {
    fs::remove(filename_);
  }
  fs::rename(tmp, filename_);
}
