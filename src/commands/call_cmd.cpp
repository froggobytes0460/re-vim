#include <algorithm>
#include <cctype>
#include <re-vim/commands.hpp>
#include <string_view>

[[nodiscard]] auto searchForCmd(std::string_view cmd) -> CmdMatch {
  const auto &entry_array = getCommandsArray();

  // Getting the final character of the command name.
  const auto *name_end = std::ranges::find_if(
      cmd, [](char l) -> bool { return std::isalpha(l) == 0; });

  const auto NAME_LEN = static_cast<size_t>(name_end - cmd.begin());
  std::string_view name = cmd.substr(0, NAME_LEN);

  // Binary searches through the array.
  auto iter = std::ranges::lower_bound(
      entry_array, name, {},
      [](const CmdEntry &entry) -> std::string_view { return entry.name; });

  for (; iter != entry_array.end() && iter->name.starts_with(name); ++iter) {
    if (name.size() >= iter->namelen) {
      return {.entry = &(*iter), .consumed = NAME_LEN};
    }
  }

  return {};
}

void callCmd(std::string_view cmd) {
  CmdMatch match = searchForCmd(cmd);
  if (match.entry == nullptr) {
    return;
  }

  Command cmd_struct{};

  if (cmd.at(match.consumed - 1) == '!') {
    cmd_struct.forceit = true;
    cmd_struct.arg = cmd.substr(match.consumed + 1);
  } else {
    cmd_struct.arg = cmd.substr(match.consumed);
  }

  if (match.entry->handler == nullptr) {
    return;
  }
  match.entry->handler(cmd_struct);
}
