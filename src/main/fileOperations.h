#ifndef FILEOPERATIONS_H
#define FILEOPERATIONS_H

#include <0_GlobalIncludes.h>
#include <easycringelib.h>
#include <threads.h>
/// \brief Makes a thread to save data to a file
struct saveToFileArgs {
  su64 content;
  const char *filename;
};

int saveToFile(struct saveToFileArgs *args);

// Makes a thread to save data to a file
// 
thrd_t saveToFileAsync(struct saveToFileArgs *args);

#endif
