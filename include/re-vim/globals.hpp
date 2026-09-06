#pragma once

#include <re-vim/buffer.hpp>

namespace global_vars {
/// @brief Flag to control main app loop.
/// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
inline bool f_should_quit{false};

/// @brief Active buffer, set by main() so command handlers can reach it.
/// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
inline Buffer *g_buffer{nullptr};
} // namespace global_vars
