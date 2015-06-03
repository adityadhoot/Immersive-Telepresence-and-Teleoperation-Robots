/**
 * \file DebugUtility.h
 * \author Billy Jun
 * \brief debugging-related utility functions and macros
 */
#ifndef DEBUG_UTILITY_H
#define DEBUG_UTILITY_H

// This is the standard macro for disabling debug message in POSIX OS.
#ifdef NDEBUG

#include <iostream>

// Replace the logger calls with no-operations
#define Logger std
#define MaxLevel cout
#define setOutDescriptor(x) cout
#define setErrDescriptor(x) cout
#define out(x,y,...) cout
#define err(x,y,...) cout
#define setLevel(x) cout
#define level cout
#define WTF_START
#define WTF_HANDLE(...)
#define WTF_END

#else

// Logger class declaraction
#include <exception>
#include <iostream>

// Declare all functions as static members so that users can call this
// function without instantiating the class.
class Logger
{
  private:
    static FILE* OutFD;
    static FILE* ErrFD;
    static unsigned Level;

  public:
    static const unsigned MaxLevel;

  public:
    static void setOutDescriptor(FILE* _outFD);
    static void setErrDescriptor(FILE* _errFD);

    static void out(int level, const char* fmt, ...);
    static void err(int level, const char* fmt, ...);

    static void setLevel(unsigned level);
    static unsigned level();
};

// Work-time fun macros to make work a bit more pleasant
#define WTF_START try {
#define WTF_HANDLE(...) \
    } catch (std::exception e) { \
    Logger::err(Logger::MaxLevel, "Caught exception: %s\n", e.what()); \
    return __VA_ARGS__; \
    } catch (...) {
#define WTF_END }

#endif

#endif
