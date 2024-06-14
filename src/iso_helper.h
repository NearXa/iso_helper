#ifndef ISO_HELPER_H
#define ISO_HELPER_H

#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "iso9660.h"

#define MAX_PATH_LEN 2048

void print_help();
void info(const struct iso_prim_voldesc *vd);
void print_directory_entry(struct iso_dir *entry, bool first);
void ls(struct iso_dir *dir, void *iso_data);
void find_directory_entry(struct iso_dir **current_dir, const char *path,
                          void *iso_data, uint32_t extent, uint32_t size);
void cd(struct iso_dir **current_dir, const char *path, char *current_path,
        struct iso_dir *root_dir, void *iso_data);
void pwd(const char *current_path);
void copy_file(struct iso_dir *entry, void *iso_data);
void get(struct iso_dir *dir, const char *path, void *iso_data);
void cat(struct iso_dir *dir, const char *path, void *iso_data);

#endif // ISO_HELPER_H
