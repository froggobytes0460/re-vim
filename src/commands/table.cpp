#include <cstddef>
#include <re-vim/commands.hpp>

namespace {
void quitCmd(Command & /*cmd*/) { global_flag::f_should_quit = true; }
} // namespace

auto getCommandsArray() -> std::span<const CmdEntry> {
  /// @brief Array of commands supported by this text editor.
  /// @note Ensure this array is **sorted**, as the search algorithm uses
  /// binary search.
  static const std::array COMMANDS{CmdEntry{.name = "qall",
                                            .namelen = 2,
                                            .handler = quitCmd,
                                            .flags = FlagCmd::Notrlbar,
                                            .addr_type = AddrType::NONE},
                                   CmdEntry{.name = "quit",
                                            .namelen = 1,
                                            .handler = quitCmd,
                                            .flags = FlagCmd::Notrlbar,
                                            .addr_type = AddrType::NONE},
                                   CmdEntry{.name = "quitall",
                                            .namelen = 2,
                                            .handler = quitCmd,
                                            .flags = FlagCmd::Notrlbar,
                                            .addr_type = AddrType::NONE}};
  return COMMANDS;
}
