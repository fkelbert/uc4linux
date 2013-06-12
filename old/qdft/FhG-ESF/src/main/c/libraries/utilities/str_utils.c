/*
 * str_utils.c
 *
 *      Author: Ricardo Neisse, Cornelius Moucha
 */

#include "utilities/str_utils.h"
#include "log_str_utils_c.h"

int substr(char *str1, char *str2)
{
  int len1, len2, i=0, j=0, k, n;
  len1=strlen(str1); //determine the lengths of the strings
  len2=strlen(str2);
  while(i < len1)
  {
    while(str2[j] == str1[i])
    {
      for(k=1, n=i + 1; k < len2; k++, n++)
      {
        //n is assigned i, which is where the match //starts in string 1- starting at the next letter
        if(str2[k] != str1[n]) return FALSE; //the match fails after having made an initial match
      }
      return TRUE;
      j++;
    }
    i++;
  }
  return FALSE;
}

int count_non_empty(char *s)
{
  int j, c=0;
  if(s == NULL) return c;
  for(j=0; j < strlen(s); j++)
  {
    if(!((s[j] == ' ') || (s[j] == '\t') || (s[j] == '\r') || (s[j] == '\n')))
      c++;
  }
  return c;
}

void trim(char *s)
{
  int i=0, j;
  while((s[i] == ' ') || (s[i] == '\t') || (s[i] == '\r') || (s[i] == '\n'))
    i++;

  if(i > 0)
  {
    for(j=0; j < strlen(s); j++)
      s[j]=s[j + i];
    s[j]='\0';
  }
  // Trim spaces and tabs from end:
  i=strlen(s) - 1;
  while((s[i] == ' ') || (s[i] == '\t') || (s[i] == '\r') || (s[i] == '\n'))
    i--;

  if(i < (strlen(s) - 1))
    s[i + 1]='\0';
}

unsigned char *byteToBinary(unsigned int x)
{
  static unsigned char b[17];
  int i;
  for(i=15; i >= 0; i--)
    b[15 - i]=(x >> i & 1) + 48;
  b[16]='\0';
  return b;
}

