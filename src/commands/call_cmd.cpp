#include <algorithm>
#include <cctype>
#include <charconv>
#include <cstdint>
#include <re-vim/buffer.hpp>
#include <re-vim/commands.hpp>
#include <re-vim/cursor.hpp>
#include <stdexcept>
#include <string_view>
#include <utility>

namespace {
/// @brief Parses a single address (digits, `.`, or `$`) at the front of
/// `cmd`.
/// @return Resolved line number (1-based, or -1 if no address was present),
/// and the number of chars consumed from the front of `cmd`.
auto parseAddr(std::string_view cmd, const Cursor &cursor, const Buffer &buffer)
    -> std::pair<int, size_t> {
  if (cmd.empty()) {
    return {-1, 0};
  }

  if (cmd.front() == '.') {
    return {cursor.line() + 1, 1};
  }

  if (cmd.front() == '$') {
    return {static_cast<int>(buffer.getLines().size()), 1};
  }

  if (std::isdigit(static_cast<uint8_t>(cmd.front())) == 0) {
    return {-1, 0};
  }

  int value{};
  auto [ptr, ec] = std::from_chars(cmd.data(), cmd.data() + cmd.size(), value);
  if (ec != std::errc{}) {
    return {-1, 0};
  }
  return {value, static_cast<size_t>(ptr - cmd.data())};
}

/// @brief Parses a leading range (`1,5`, `.`, `$`, `%`) from `cmd`.
/// @return Parsed range, and the number of chars consumed from the front of
/// `cmd`.
auto parseRange(std::string_view cmd, const Cursor &cursor,
                const Buffer &buffer) -> std::pair<Range, size_t> {
  if (!cmd.empty() && cmd.front() == '%') {
    return {{.line1 = 1,
             .line2 = static_cast<int>(buffer.getLines().size()),
             .addr_count = 2,
             .valid = true},
            1};
  }

  auto [addr1, len1] = parseAddr(cmd, cursor, buffer);
  if (addr1 == -1) {
    return {{}, 0};
  }
  size_t pos = len1;

  if (pos >= cmd.size() || cmd.at(pos) != ',') {
    return {{.line1 = addr1, .line2 = addr1, .addr_count = 1, .valid = true},
            pos};
  }

  ++pos; // Skip ','.
  auto [addr2, len2] = parseAddr(cmd.substr(pos), cursor, buffer);
  if (addr2 == -1) {
    throw std::invalid_argument("range: expected address after ','");
  }
  pos += len2;

  return {{.line1 = addr1, .line2 = addr2, .addr_count = 2, .valid = true},
          pos};
}
} // namespace

auto searchForCmd(std::string_view cmd, const Cursor &cursor,
                  const Buffer &buffer) -> CmdMatch {
  const auto &entry_array = getCommandsArray();

  auto [range, pos] = parseRange(cmd, cursor, buffer);

  // Getting the first non-alphabetical character of the command string.
  const auto *name_end =
      std::find_if(cmd.begin() + static_cast<std::ptrdiff_t>(pos), cmd.end(),
                   [](char l) -> bool {
                     return std::isalpha(static_cast<unsigned char>(l)) == 0;
                   });

  const auto NAME_LEN = static_cast<size_t>(name_end - cmd.begin()) - pos;
  std::string_view name = cmd.substr(pos, NAME_LEN);

  // Binary searches through the array.
  auto iter = std::ranges::lower_bound(
      entry_array, name, {},
      [](const CmdEntry &entry) -> std::string_view { return entry.name; });

  for (; iter != entry_array.end() && iter->name.starts_with(name); ++iter) {
    if (name.size() >= iter->namelen) {
      return {.range = range, .entry = &(*iter), .consumed = pos + NAME_LEN};
    }
  }

  return {.range = range, .entry = nullptr, .consumed = 0};
}

auto constructCmdStruct(std::string_view cmd, CmdMatch match) -> Command {
  Command cmd_struct{};
  const uint32_t FLAGS = match.entry->flags;

  if (match.range.valid) {
    if ((FLAGS & FlagCmd::RangeCmd) == 0) {
      throw std::invalid_argument("command does not accept a range");
    }
    cmd_struct.line1 = match.range.line1;
    cmd_struct.line2 = match.range.line2;
    cmd_struct.addr_count = match.range.addr_count;
  }

  size_t pos = match.consumed;

  if (pos < cmd.size() && cmd.at(pos) == '!') {
    if ((FLAGS & FlagCmd::Bang) == 0) {
      throw std::invalid_argument("command does not accept '!'");
    }
    cmd_struct.forceit = true;
    ++pos;
  }

  if ((FLAGS & FlagCmd::Extra) != 0 && pos < cmd.size() &&
      std::isspace(static_cast<unsigned char>(cmd.at(pos))) == 0) {
    cmd_struct.extra = cmd.at(pos);
    ++pos;
  }

  while (pos < cmd.size() &&
         std::isspace(static_cast<unsigned char>(cmd.at(pos))) != 0) {
    ++pos;
  }

  size_t count_start = pos;
  while (pos < cmd.size() &&
         std::isdigit(static_cast<unsigned char>(cmd.at(pos))) != 0) {
    ++pos;
  }
  if (pos > count_start) {
    int count{};
    std::from_chars(cmd.data() + count_start, cmd.data() + pos, count);
    cmd_struct.count = count;

    while (pos < cmd.size() &&
           std::isspace(static_cast<unsigned char>(cmd.at(pos))) != 0) {
      ++pos;
    }
  }

  cmd_struct.arg = cmd.substr(pos);

  return cmd_struct;
}
