/**
 * \file DebugUtility.cpp
 * \author Billy Jun
 * \brief debugging-related utility function definitions
 */
#ifndef NDEBUG

#include <iostream>
#include <stdio.h>
#include <stdarg.h>
#include "DebugUtility.h"

FILE* Logger::OutFD = stdout;
FILE* Logger::ErrFD = stderr;
unsigned Logger::Level = 0;
const unsigned Logger::MaxLevel = 0xFFFFFFFF;

void Logger::setOutDescriptor(FILE* _outFD)
{
    OutFD = _outFD;
}

void Logger::setErrDescriptor(FILE* _errFD)
{
    ErrFD = _errFD;
}

void Logger::out(int level, const char* fmt, ...)
{
    if (level > Level)
    {
        va_list ap;
        va_start(ap, fmt);
        vfprintf(OutFD, fmt, ap);
        va_end(ap);
    }
}

void Logger::err(int level, const char* fmt, ...)
{
    if (level > Level)
    {
        va_list ap;
        va_start(ap, fmt);
        vfprintf(ErrFD, fmt, ap);
        va_end(ap);
    }
}

void Logger::setLevel(unsigned level)
{
    Level = level;
}

unsigned Logger::level()
{
    return Level;
}

#endif
