#include "fileOperations.h"

#define Le(...)                                                                \
  printf("\033[0;31m%s:%s:%d: ", __FILE__, __func__, __LINE__);                \
  printf(__VA_ARGS__);                                                         \
  printf("\033[0m\n");

int saveToFile(struct saveToFileArgs *args) {
  FILE *file = fopen(args->filename, "w");
  if (file == NULL) {
    Le("Error opening file %s", args->filename);
    return 1;
  }
  fprintf(file, "%.*s", (i32)args->content.size, args->content.data);
  fclose(file);
  return 0;
}

int _saveToFile(void *args) {
  return saveToFile((struct saveToFileArgs *)args);
}

thrd_t saveToFileAsync(struct saveToFileArgs *args) {
  thrd_t thread;
  thrd_create(&thread, (thrd_start_t)_saveToFile, (void *)args);
  return thread;
}
