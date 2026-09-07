#pragma once

#include <re-vim/buffer.hpp>
#include <re-vim/modes.hpp>

namespace global_vars {
/// @brief Flag to control termination of program.
// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
inline bool termination_flag{false};

/// @brief Active buffer, set by main() so command handlers can reach it.
// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
inline Buffer *g_buffer{nullptr};

/// @brief Cursor mode.
// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
inline Mode mode{Mode::NORMAL};
} // namespace global_vars
