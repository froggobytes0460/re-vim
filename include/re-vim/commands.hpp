#pragma once

#include <cstdint>
#include <functional>
#include <span>
#include <string>

class Buffer;
class Cursor;

/// @brief Character that opens the command-line prompt (`:`).
constexpr char COMMAND_KEY{':'};

struct Command;

/// @brief Flags vim commands can have.
enum FlagCmd : uint16_t {
  RangeCmd = 0x001, ///< Command accepts a line range (e.g. `:1,5d`).
  Bang = 0x002,     ///< Command accepts a `!` modifier (e.g. `:q!`).
  Extra = 0x004, ///< Command accepts extra (non-argument) chars after its name.
  Xfile = 0x008, ///< Argument is a filename, subject to filename expansion.
  Needarg = 0x080,  ///< Command requires at least one argument.
  Trlbar = 0x100,   ///< Trailing `|` may chain another command after this one.
  Notrlbar = 0x200, ///< Trailing `|` is not allowed after this command.
};

/// @brief Kind of address a command's range refers to.
enum class AddrType : uint8_t {
  NONE,    ///< Command takes no address/range.
  LINES,   ///< Range addresses lines in the buffer.
  BUFFERS, ///< Range addresses buffer numbers.
  WINDOWS, ///< Range addresses window numbers.
  TABS,    ///< Range addresses tab page numbers.
};

/// @brief Callback invoked to execute a matched command.
using CmdHandler = std::function<void(Command &)>;

/// @brief Static definition of a known ex command.
struct CmdEntry {
  std::string name;     ///< Full command name (e.g. "write").
  size_t namelen{};     ///< Minimum unambiguous prefix length for @ref name.
  CmdHandler handler;   ///< Function that executes the command.
  uint32_t flags{};     ///< Bitmask of @ref FlagCmd values.
  AddrType addr_type{}; ///< Kind of range this command accepts.
};

/// @brief Parsed instance of a command line, ready for execution.
struct Command {
  // Range
  int line1{0};      ///< First line of the range.
  int line2{0};      ///< Last line of the range.
  int addr_count{0}; ///< Number of addresses supplied in the range.
  int count{0};      ///< Numeric count argument, if any (e.g. `:next 3`).

  // Modifiers
  bool forceit{false}; ///< Whether the `!` modifier was present.
  char extra{0};       ///< Extra modifier character, if any.

  // Argument
  std::string arg; ///< Remaining argument text after the command name.
};

/// @brief Parsed line range (e.g. `1,5`) before resolution against a buffer.
struct Range {
  int line1{0};      ///< First line of the range.
  int line2{0};      ///< Last line of the range.
  int addr_count{0}; ///< Number of addresses supplied (0, 1, or 2).
  bool valid{false}; ///< Whether parsing produced a valid range.
};

/// @brief Result of matching input text against known @ref CmdEntry
/// definitions.
struct CmdMatch {
  const CmdEntry *entry =
      nullptr;       ///< Matched command definition, or nullptr if none.
  size_t consumed{}; ///< Chars of input eaten by the range and name.
  Range range;       ///< Range parsed before the command name, if any.
};

/// @brief Get pre-computed array of commands.
[[nodiscard]] auto getCommandsArray() noexcept -> std::span<const CmdEntry>;

/// @brief Reads command-line input from the user after @ref COMMAND_KEY is
/// pressed.
/// @return Text entered by the user, excluding the leading @ref COMMAND_KEY.
[[nodiscard]] auto readCommandLine() noexcept -> std::string;

/// @brief Searches for command, parsing a leading range (`1,5`, `.`, `$`,
/// `%`) if present.
/// @param[in] cmd User string to find command from.
/// @param[in] cursor Used to resolve `.` to the current line.
/// @param[in] buffer Used to resolve `$`/`%` to the last line.
/// @return Command entry found (nullptr if not found), chars of input
/// consumed by the range and name, and the parsed range (if any).
[[nodiscard]] auto searchForCmd(std::string_view cmd, const Cursor &cursor,
                                const Buffer &buffer) -> CmdMatch;

/// @brief Constucts a proper command struct from command, used to pass to
/// command handler.
/// @param[in] cmd The command string.
/// @param[in] match The match found from searching for command in command
/// entires array.
/// @return Command struct including necessary data to run the command handler.
[[nodiscard]] auto constructCmdStruct(std::string_view cmd, CmdMatch match)
    -> Command;
