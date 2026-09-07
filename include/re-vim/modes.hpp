#pragma once

#include <cstdint>

/// @brief Vim-like modes, used for controlling navigation, insertions and
/// deletions.
enum class Mode : uint8_t { NORMAL, INSERT };
