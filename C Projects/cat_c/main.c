#include "stdio.h"
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 1024

void cat(const char *path)
{
  printf("\n====================== %s ==========================\n", path);

  FILE *fptr = fopen(path, "rb");
  if (fptr == NULL)
  {
    perror(path);
    return;
  }
  char buffers[BUFFER_SIZE];
  size_t nitems; // num of bytes to read

  while ((nitems = fread(buffers, 1, BUFFER_SIZE, fptr)) > 0)
  {
    if (fwrite(buffers, 1, nitems, stdout) != nitems)
    {
      perror("stdout");
      break;
    }
  }

  if (ferror(fptr))
    perror(path);

  fclose(fptr);
}

int is_ignore(const char *path)
{
  // ignore . file
  const char *name = strrchr(path, '/');
  if (name)
    name++;
  else
    name = path;

  // igonor all file or folder start with .
  return name[0] == '.';
}

void walkcat(const char *path)
{
  if (is_ignore(path))
    return;
  struct stat st;
  if (lstat(path, &st) == -1)
  {
    perror(path);
    return;
  }
  if (S_ISREG((st.st_mode)))
  {
    // if file = > cat
    cat(path);
    return;
  }

  if (!S_ISDIR(st.st_mode))
    return;

  // is folder
  DIR *dir = opendir(path);
  if (!dir)
  {
    perror(path);
    return;
  }
  struct dirent *dp;
  while ((dp = readdir(dir)) != NULL)
  {
    const char *name = dp->d_name; // get name
    if (is_ignore(name))
      continue;
    // make new sub_path
    size_t len = strlen(path) + strlen(name) + 2;
    char sub_path[len];
    if (strcmp(path, "/") == 0)
      snprintf(sub_path, len, "/%s", name);
    else
      snprintf(sub_path, len, "%s/%s", path, name);
    walkcat(sub_path);
  }
  closedir(dir);
}

int main(int argc, char *argv[])
{
  if (argc == 1)
  {
    printf("Usage : %s <filename>.. \n", argv[0]);
    return 0;
  }
  for (int i = 1; i < argc; i++)
  {
    const char *path = argv[i];
    walkcat(path);
  }
}