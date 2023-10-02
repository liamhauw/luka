/*
  SPDX license identifier: MIT

  Copyright (c) 2023 Liam Hauw

  Common header file.
*/

#pragma once

namespace luka
{
  
#ifdef _WIN32
#define PATH_SEPARATOR '\\'
#elif __APPLE__
#define PATH_SEPARATOR '/'
#endif

} // namespace luka
