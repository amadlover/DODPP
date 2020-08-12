#include "utils.h"
#include <Shlwapi.h>
#include <strsafe.h>

#pragma comment (lib, "Shlwapi.lib")

#define STB_IMAGE_IMPLEMENTATION
#define STB_ONLY_PNG
#define STB_ONLY_TGA

#include <stb_image.h>

void utils_get_full_texture_path_from_uri (const char* file_path, const char* uri, char* out_full_texture_path)
{
	wchar_t texture_file[MAX_PATH];
	mbstowcs (texture_file, file_path, MAX_PATH);

	PathRemoveFileSpec (texture_file);
	wchar_t t_uri[MAX_PATH];
	mbstowcs (t_uri, uri, MAX_PATH);

	wchar_t uri_path[MAX_PATH];
	StringCchCopy (uri_path, MAX_PATH, L"\\");
	StringCchCat (uri_path, MAX_PATH, t_uri);

	StringCchCat (texture_file, MAX_PATH, uri_path);
	wcstombs (out_full_texture_path, texture_file, MAX_PATH);
}

void utils_get_full_file_path (const char* partial_file_path, char* out_file_path)
{
	char path[MAX_PATH];

	wchar_t t_path[MAX_PATH];
	HMODULE module = GetModuleHandle (NULL);
	GetModuleFileName (module, t_path, MAX_PATH);
	PathRemoveFileSpec (t_path);

	wcstombs_s (NULL, path, MAX_PATH, t_path, MAX_PATH);
	strcpy (out_file_path, path);
	strcat (out_file_path, "\\");
	strcat (out_file_path, partial_file_path);
}

void utils_get_files_in_folder (const char* partial_folder_path, file_path** out_file_paths, size_t* num_out_files)
{
	char full_folder_path[MAX_PATH];
	utils_get_full_file_path (partial_folder_path, full_folder_path);
	strcat (full_folder_path, "*");

	wchar_t folder_path[MAX_PATH];
	mbstowcs (folder_path, full_folder_path, MAX_PATH);

	WIN32_FIND_DATA ffd;
	HANDLE find_handle = INVALID_HANDLE_VALUE;

	find_handle = FindFirstFile (folder_path, &ffd);
	size_t num_files = 0;
	do
	{
		if (!(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
		{
			char file_name[MAX_PATH];
			wcstombs (file_name, ffd.cFileName, MAX_PATH);
			char* base_name = strtok (file_name, ".");
			char* ext = strtok (NULL, ".");

			if (strcmp (ext, "glb") == 0 || strcmp (ext, "gltf") == 0)
			{
				++num_files;
			}
		}
	} while (FindNextFile (find_handle, &ffd) != 0);

	*num_out_files = num_files;
	*out_file_paths = (file_path*)utils_calloc (num_files, sizeof (file_path));

	find_handle = FindFirstFile (folder_path, &ffd);
	size_t current_file_index = 0;
	do
	{
		if (!(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
		{
			char file_name[MAX_PATH];
			wcstombs (file_name, ffd.cFileName, MAX_PATH);
			char* base_name = strtok (file_name, ".");
			char* ext = strtok (NULL, ".");

			if (strcmp (ext, "glb") == 0 || strcmp (ext, "gltf") == 0)
			{
				file_path* current_file_path = *out_file_paths + current_file_index;
				strcpy (current_file_path->path, base_name);
				strcat (current_file_path->path, ".");
				strcat (current_file_path->path, ext);
				++current_file_index;
			}
		}
	} while (FindNextFile (find_handle, &ffd) != 0);
}

void utils_import_texture (const char* texture_path, int* width, int* height, int* bpp, uint8_t** pixels)
{
	char full_path[MAX_PATH];
	utils_get_full_file_path (texture_path, full_path);
	*pixels = stbi_load (full_path, width, height, bpp, 4);
}

void* utils_malloc (const size_t size)
{
	return malloc (size);
}

void* utils_malloc_zero (const size_t size)
{
	void* ptr = malloc (size);
	memset (ptr, 0, size);
	return ptr;
}

void* utils_aligned_malloc (const size_t size, const size_t alignment)
{
	return _aligned_malloc (size, alignment);
}

void* utils_aligned_malloc_zero (const size_t size, const size_t alignment)
{
	void* ptr = _aligned_malloc (size, alignment);
	memset (ptr, 0, size);

	return ptr;
}

void* utils_calloc (const size_t count, const size_t size)
{
	return calloc (count, size);
}

void* utils_aligned_calloc (const size_t count, const size_t size, const size_t alignment)
{
	void* ptr = _aligned_malloc (size * count, alignment);
	memset (ptr, 0, size * count);

	return ptr;
}

void* utils_realloc (void* ptr, size_t new_size)
{
	void* new_ptr = realloc (ptr, new_size);
	return new_ptr;
}

void* utils_realloc_zero (void* ptr, size_t old_size, size_t new_size)
{
	void* new_ptr = realloc (ptr, new_size);
	memset ((char*)new_ptr + old_size, 0, new_size - old_size);
	return new_ptr;
}

void* utils_aligned_realloc_zero (void* ptr, size_t alignment, size_t old_size, size_t new_size)
{
	void* new_ptr = _aligned_realloc (ptr, new_size, alignment);
	memset ((char*)new_ptr + old_size, 0, new_size - old_size);
	return new_ptr;
}

void utils_free (void* ptr)
{
	if (ptr != NULL)
	{
		free (ptr);
		ptr = NULL;
	}
}

void utils_aligned_free (void* ptr)
{
	if (ptr != NULL)
	{
		_aligned_free (ptr);
		ptr = NULL;
	}
}

void utils_free_image_data (uint8_t * pixels)
{
	stbi_image_free (pixels);
}