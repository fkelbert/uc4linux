/**
 * @file  stringUtils.c
 * @brief auxiliary methods for string processing
 *
 * @author cornelius moucha, Ricardo Neisse
 **/

#include "stringUtils.h"
#include "log_stringUtils_pef.h"

char *byteToBinary(unsigned int x)
{
  static char b[17];
  int i;
  for(i=15; i >= 0; i--)
    b[15 - i]=(x >> i & 1) + 48;
  b[16]='\0';
  return b;
}

