#include <algorithm>
#include <cctype>
#include <re-vim/commands.hpp>
#include <string_view>

auto searchForCmd(std::string_view cmd) noexcept -> CmdMatch {
  const auto &entry_array = getCommandsArray();

  // Getting the first non-alphabetical character of the command string.
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

auto constructCmdStruct(std::string_view cmd, CmdMatch match) -> Command {
  Command cmd_struct{};

  if (cmd.at(match.consumed - 1) == '!') {
    cmd_struct.forceit = true;
    cmd_struct.arg = cmd.substr(match.consumed + 1);
  } else {
    cmd_struct.arg = cmd.substr(match.consumed);
  }

  return cmd_struct;
}
