#pragma once

#include <ncurses.h>
#include <re-vim/buffer.hpp>
#include <re-vim/modes.hpp>

namespace global_vars {
/// @brief Flag to control termination of program.
// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
inline bool termination_flag{false};

/// @brief Active buffer, set by main() so command handlers can reach it.
// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
inline Buffer *curr_buffer{nullptr};

/// @brief Status line window, set by main() so command handlers (e.g. the
/// `:` command prompt) can draw into it without colliding with the text
/// window.
// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
inline WINDOW *status_win{nullptr};

/// @brief Cursor mode.
// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
inline Mode mode{Mode::NORMAL};
} // namespace global_vars
