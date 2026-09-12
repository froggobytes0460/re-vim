#include <cstddef>
#include <re-vim/buffer.hpp>
#include <re-vim/commands.hpp>
#include <re-vim/globals.hpp>

namespace {
void quitCmd(const Command &cmd) {
  if (!global_vars::curr_buffer->isSyncronized() && !cmd.forceit) {
    return;
  }
  global_vars::termination_flag = true;
}

void writeCmd(Command & /*cmd*/) {
  if (global_vars::curr_buffer != nullptr) {
    global_vars::curr_buffer->save();
  }
}
} // namespace

auto getCommandsArray() noexcept -> std::span<const CmdEntry> {
  // NOTE: This should always remain sorted, as the search uses binary search
  // algorithm.
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
