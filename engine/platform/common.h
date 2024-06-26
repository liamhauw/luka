// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

#pragma once

#include <cstdint>

namespace luka {

using i8 = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;

using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

using f32 = float;
using f64 = double;

#ifdef _WIN32
#define PATH_SEPARATOR '\\'
#elif __APPLE__
#define PATH_SEPARATOR '/'
#endif

}  // namespace luka
