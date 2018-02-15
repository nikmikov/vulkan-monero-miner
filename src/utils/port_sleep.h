#pragma once

#ifdef _WIN32
#include <windows.h>

static inline void port_sleep(size_t sec) { Sleep(sec * 1000); }
#else
#include <unistd.h>

static inline void port_sleep(size_t sec) { sleep(sec); }
#endif // _WIN32
