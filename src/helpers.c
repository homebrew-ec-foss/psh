// helpers.c
#include "psh.h"


void remove_last_component(char *path)
{
  char *last_slash = strrchr(path, '/');
  if (last_slash != NULL)
  {
    *last_slash = '\0';
  }
}

int compare_strings(const void *a, const void *b)
{
  return strcmp(*(const char **)a, *(const char **)b);
}

void sort_strings(char **strings, int num_strings)
{
  qsort(strings, num_strings, sizeof(char *), compare_strings);
}
