/**
 * @file assert.h
 * @brief Runtime assertion and panic handling.
 *
 * This file provides macros for runtime checks (ELIB_ASSERT) and unconditional
 * failures (ELIB_PANIC). It calls std::abort() by default, which works on
 * both host (terminates process) and embedded (links to system exit handler).
 *
 * Configuration Macros (define in elib.tweaks.h):
 * - ELIB_CONFIG_NO_ASSERTS: If defined, ALL assertions (including critical) are compiled out.
 * - ELIB_USER_ERROR_HANDLER: If defined, the user must implement `elib::onError`.
 * - NDEBUG: Standard C++ macro. If defined, ELIB_ASSERT_DEBUG is compiled out.
 */

#pragma once

#include <elib/config.h>
#include <cstdlib> // for std::abort

namespace elib
{
#if defined(ELIB_USER_ERROR_HANDLER)
  /**
   * @brief User-defined error handler.
   * @note Must be implemented in application code if ELIB_USER_ERROR_HANDLER is defined.
   */
  extern void onError(const char* file, int line, const char* message);

#else
  /**
   * @brief Default portable error handler.
   * Calls std::abort(), which terminates the process on Host
   * and calls the system exit handler (e.g. _exit) on Embedded.
   */
  inline void onError(const char* /*file*/, int /*line*/, const char* /*message*/)
  {
    std::abort();
  }
#endif
}

// --------------------------------------------------------------------------
// 1. Critical Assert Macros (Always Active unless ELIB_CONFIG_NO_ASSERTS)
// --------------------------------------------------------------------------

#if defined(ELIB_CONFIG_NO_ASSERTS)

  #define ELIB_ASSERT(condition, message) ((void)0)
  #define ELIB_PANIC(message)             ((void)0)

#else

  #define ELIB_ASSERT(condition, message) \
    do { \
      if (!(condition)) { \
        ::elib::onError(__FILE__, __LINE__, message); \
      } \
    } while (0)

  #define ELIB_PANIC(message) \
    ::elib::onError(__FILE__, __LINE__, message)

#endif

// --------------------------------------------------------------------------
// 2. Debug Assert Macros (Active only in Debug Builds)
// --------------------------------------------------------------------------
// Enabled only if:
// 1. ELIB_CONFIG_NO_ASSERTS is NOT defined (Master Kill-Switch)
// 2. NDEBUG is NOT defined (Standard Release Switch)

#if !defined(ELIB_CONFIG_NO_ASSERTS) && !defined(NDEBUG)
  #define ELIB_ASSERT_DEBUG(condition, message) ELIB_ASSERT(condition, message)
#else
  #define ELIB_ASSERT_DEBUG(condition, message) ((void)0)
#endif