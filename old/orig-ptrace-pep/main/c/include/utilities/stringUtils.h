/**
 * @file  stringUtils.h
 * @brief Auxiliary methods for string processing
 *
 * @author cornelius moucha
 **/

#ifndef stringUtils_h
#define stringUtils_h

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "base.h"

/**
 * Auxiliary method: Converts a byte to a string representation (used in systrace-PEP)
 * @param   x      the byte for converting
 * @return  the string representation for the byte
 */
char *byteToBinary(unsigned int x);

#endif
