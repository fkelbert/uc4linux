/**
 * @file  exRegex.c
 * @brief Exemplary example for regular expressions
 *
 * @author cornelius moucha
**/

#include <stdio.h>
#include <regex.h>
#include <stdlib.h>

int main()
{
  printf("Regex example\n");
  regex_t regex;
  int reti;
  char msgbuf[100];

  // precompile regular expression
  reti=regcomp(&regex, "event.homeapp.controller.frontend/[a-zA-Z0-9]*", 0);
  if(reti) {fprintf(stderr, "Could not compile regex"); exit(1);}

  // Execute regular expression matching
  reti=regexec(&regex, "event.homeapp.controller.frontend/QcmnthBumNXekSZo", 0, NULL, 0);
  if(!reti) printf("RE match\n");
  else if(reti==REG_NOMATCH) printf("No match\n");
  else
  {
    regerror(reti, &regex, msgbuf, sizeof(msgbuf));
    fprintf(stderr, "Regex match failed: %s\n", msgbuf);
    exit(1);
  }
  regfree(&regex);
  return 0;
}
