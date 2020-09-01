#ifndef UTILS_HPP
#define UTILS_HPP

#include <Windows.h>
#include <cstdbool>
#include <cstdint>

#define CHECK_AGAINST_RESULT(func, result) result = func; if (result != 0) return result;

typedef struct _file_path
{
    char path[MAX_PATH];
} file_path;

void utils_get_full_file_path (const char* partial_file_path, char* out_file_path);
void utils_get_files_in_folder (const char* partial_folder_path, file_path** out_file_paths, uint32_t* num_out_files);

void utils_import_texture (const char* file_path, int* width, int* height, int* bpp, uint8_t** pixels);

void* utils_malloc (const uint32_t size);
void* utils_malloc_zero (const uint32_t size);
void* utils_aligned_malloc (const uint32_t size, const uint32_t alignment);
void* utils_aligned_malloc_zero (const uint32_t size, const uint32_t alignment);
void* utils_calloc (const uint32_t count, const uint32_t size);
void* utils_aligned_calloc (const uint32_t count, const uint32_t size, const uint32_t alignment);
void* utils_realloc (void* ptr, uint32_t new_size);
void* utils_realloc_zero (void* ptr, uint32_t old_size, uint32_t new_size);
void* utils_aligned_realloc_zero (void* ptr, uint32_t alignment, uint32_t old_size, uint32_t new_size);

void utils_free (void* ptr);
void utils_aligned_free (void* ptr);
void utils_free_image_data (uint8_t* pixels);

#endif