#include <cstddef>
#include <re-vim/buffer.hpp>
#include <re-vim/commands.hpp>
#include <re-vim/globals.hpp>

namespace {
void quitCmd(const Command &cmd) {
  if (global_vars::curr_buffer->isSyncronized() || cmd.forceit) {
    global_vars::termination_flag = true;
  }
}

void writeCmd(Command & /*cmd*/) {
  if (global_vars::curr_buffer != nullptr) {
    global_vars::curr_buffer->save();
  }
}
} // namespace

// NOLINTBEGIN(bugprone-exception-escape) None of the names are large enough
// to cause error.
auto getCommandsArray() noexcept -> std::span<const CmdEntry> {
  //  NOTE: This should always remain sorted, as the search uses binary search
  //  algorithm.
  static const std::array COMMANDS{
      CmdEntry{.name = "qall",
               .handler = quitCmd,
               .namelen = 2,
               .flags = FlagCmd::Notrlbar,
               .addr_type = AddrType::NONE},
      CmdEntry{.name = "quit",
               .handler = quitCmd,
               .namelen = 1,
               .flags = FlagCmd::Notrlbar,
               .addr_type = AddrType::NONE},
      CmdEntry{.name = "wq",
               .handler = [](Command &cmd) -> void {
                 writeCmd(cmd);
                 quitCmd(cmd);
               },
               .namelen = 2,
               .flags = FlagCmd::RangeCmd,
               .addr_type = AddrType::LINES},
      CmdEntry{.name = "write",
               .handler = writeCmd,
               .namelen = 1,
               .flags = FlagCmd::RangeCmd,
               .addr_type = AddrType::LINES}};
  return COMMANDS;
}
// NOLINTEND(bugprone-exception-escape)
