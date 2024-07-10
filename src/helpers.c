// helpers.c
#include "psh.h"

char *commonSuffix(char *str1, char *str2)
{
  int len1 = strlen(str1);
  int len2 = strlen(str2);
  int minLength = len1 < len2 ? len1 : len2;
  int i;

  // Find the length of the common suffix
  for (i = 0; i < minLength; i++)
  {
    if (str1[len1 - 1 - i] != str2[len2 - 1 - i])
    {
      break;
    }
  }

  // Allocate memory for the result string
  char *result = (char *)malloc((i + 1) * sizeof(char));
  if (result == NULL)
  {
    fprintf(stderr, "Memory allocation failed\n");
    exit(EXIT_FAILURE);
  }

  // Copy the common suffix into the result string
  strncpy(result, str1 + len1 - i, i);
  result[i] = '\0';

  return result;
}

void remove_last_component(char *path)
{
  char *last_slash = strrchr(path, '/');
  if (last_slash != NULL)
  {
    *last_slash = '\0';
  }
}

int resolve_and_manage_symlink(char *localdir, char *resolved_path)
{
  if (realpath(localdir, resolved_path) == NULL)
  {
    if (errno == ENOENT)
    {
      fprintf(stderr, "PSH: no such file or directory: %s\n", localdir);
    }
    else
    {
      perror("PSH: realpath() error");
    }
    return -1;
  }

  if (chdir(resolved_path) == -1)
  {
    perror("PSH: chdir() error");
    return -1;
  }

  return 0;
}

char *helper_cd_func1(const char *str1, const char *str2)
{
  static char result[256]; // Use static to return a pointer
  char temp1[256], temp2[256];
  char *tokens1[64], *tokens2[64];
  int len1 = 0, len2 = 0;

  // Copy the input strings to temporary variables
  strcpy(temp1, str1);
  strcpy(temp2, str2);

  // Tokenize the first string by "/"
  char *token = strtok(temp1, "/");
  while (token != NULL)
  {
    tokens1[len1++] = token;
    token = strtok(NULL, "/");
  }

  // Tokenize the second string by "/"
  token = strtok(temp2, "/");
  while (token != NULL)
  {
    tokens2[len2++] = token;
    token = strtok(NULL, "/");
  }

  // Compare tokens from the end and find the point of divergence
  int i = len1 - 1;
  int j = len2 - 1;
  while (i >= 0 && j >= 0 && strcmp(tokens1[i], tokens2[j]) == 0)
  {
    i--;
    j--;
  }

  // Build the result string from the remaining tokens in str1
  result[0] = '\0'; // Initialize result as an empty string
  for (int k = 0; k <= i; k++)
  {
    strcat(result, "/");
    strcat(result, tokens1[k]);
  }

  return result;
}
