#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stddef.h>

// Change directory
bool se_fs_chdir(const char* dirPath);
// Get current working directory
char* se_fs_get_cwd();
// Print current working directory
void se_fs_print_cwd();

size_t se_fs_get_file_size(const char* filePath);
char* se_fs_read_file_contents(const char* filePath, size_t* sz);
bool se_fs_does_file_exist(const char* filePath);

#ifdef __cplusplus
}
#endif
