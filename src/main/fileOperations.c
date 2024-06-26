#include "fileOperations.h"

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
