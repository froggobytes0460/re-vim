#include <cstddef>
#include <re-vim/buffer.hpp>
#include <re-vim/commands.hpp>
#include <re-vim/globals.hpp>

namespace {
void quitCmd(Command & /*cmd*/) { global_vars::f_should_quit = true; }

void writeCmd(Command & /*cmd*/) {
  if (global_vars::g_buffer != nullptr) {
    global_vars::g_buffer->save();
  }
}
} // namespace

auto getCommandsArray() -> std::span<const CmdEntry> {
  /// @brief Array of commands supported by this text editor.
  /// @note Ensure this array is **sorted**, as the search algorithm uses
  /// binary search.
  static const std::array COMMANDS{
      CmdEntry{.name = "qall",
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
               .addr_type = AddrType::NONE},
      CmdEntry{.name = "wq",
               .namelen = 2,
               .handler = [](Command &cmd) -> void {
                 writeCmd(cmd);
                 quitCmd(cmd);
               },
               .flags = FlagCmd::Range,
               .addr_type = AddrType::LINES},
      CmdEntry{.name = "write",
               .namelen = 1,
               .handler = writeCmd,
               .flags = FlagCmd::Range,
               .addr_type = AddrType::LINES}};
  return COMMANDS;
}
