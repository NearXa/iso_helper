#include "iso_helper.h"

void print_help()
{
    printf("help: display command help\n");
    printf("info: display volume info\n");
    printf("ls: display the content of a directory\n");
    printf("cd <dir>: change current directory\n");
    printf("get <file>: copy file to local directory\n");
    printf("cat <file>: display file content\n");
    printf("pwd: print current path\n");
    printf("quit: exit program\n");
}

void info(const struct iso_prim_voldesc *vd)
{
    printf("System Identifier: %.32s\n", vd->system_identifier);
    printf("Volume Identifier: %.32s\n", vd->volume_identifier);
    printf("Block count: %u\n", vd->volume_space_size.le);
    printf("Block size: %u\n", vd->logical_block_size.le);
    printf("Creation date: %.17s\n", vd->creation_date);
    printf("Application Identifier: %.128s\n", vd->application_id);
}

void print_directory_entry(struct iso_dir *entry, bool first)
{
    (void)first;

    char name[MAX_PATH_LEN];
    snprintf(name, entry->name_len + 1, "%s", entry->name);
    char *semicolon = strchr(name, ';');
    if (semicolon)
    {
        *semicolon = '\0';
    }

    if (entry->name_len == 1 && entry->name[0] == '\0')
    {
        strcpy(name, ".");
    }
    else if (entry->name_len == 1 && entry->name[0] == '\1')
    {
        strcpy(name, "..");
    }

    char date[21];
    if (entry->date[0] != 0)
    {
        snprintf(date, sizeof(date), "%04d/%02d/%02d %02d:%02d",
                 entry->date[0] + 1900, entry->date[1], entry->date[2],
                 entry->date[3], entry->date[4]);
    }
    else
    {
        snprintf(date, sizeof(date), "Unspecified");
    }

    printf("%c%c %9u %s %s\n", (entry->flags & ISO_FILE_ISDIR) ? 'd' : '-',
           (entry->flags & ISO_FILE_HIDDEN) ? 'h' : '-', entry->size.le, date,
           name);
}

void ls(struct iso_dir *dir, void *iso_data)
{
    uint32_t extent = dir->extent.le;
    uint32_t size = dir->size.le;
    void *dir_data = (char *)iso_data + extent * ISO_BLOCK_SIZE;
    size_t offset = 0;
    bool first = true;

    while (offset < size)
    {
        struct iso_dir *entry = (struct iso_dir *)((char *)dir_data + offset);
        if (entry->length == 0)
        {
            break;
        }
        print_directory_entry(entry, first);
        first = false;
        offset += entry->length;
    }
}

void find_directory_entry(struct iso_dir **current_dir, const char *path,
                          void *iso_data, uint32_t extent, uint32_t size)
{
    void *dir_data = (char *)iso_data + extent * ISO_BLOCK_SIZE;
    size_t offset = 0;

    while (offset < size)
    {
        struct iso_dir *entry = (struct iso_dir *)((char *)dir_data + offset);
        if (entry->length == 0)
        {
            break;
        }
        char entry_name[MAX_PATH_LEN];
        snprintf(entry_name, entry->name_len + 1, "%s", entry->name);
        char *semicolon = strchr(entry_name, ';');
        if (semicolon)
        {
            *semicolon = '\0';
        }
        if (strcmp(entry_name, path) == 0 && (entry->flags & ISO_FILE_ISDIR))
        {
            *current_dir = entry;
            return;
        }
        offset += entry->length;
    }
    fprintf(stderr, "Directory not found: %s\n", path);
}

void cd(struct iso_dir **current_dir, const char *path, char *current_path,
        struct iso_dir *root_dir, void *iso_data)
{
    if (strcmp(path, "") == 0)
    {
        *current_dir = root_dir;
        strcpy(current_path, "/");
        return;
    }

    uint32_t extent = (*current_dir)->extent.le;
    uint32_t size = (*current_dir)->size.le;
    find_directory_entry(current_dir, path, iso_data, extent, size);

    if (*current_dir != root_dir && strcmp(path, ".") != 0)
    {
        if (strcmp(current_path, "/") != 0)
        {
            strcat(current_path, "/");
        }
        strcat(current_path, path);
    }
}

void pwd(const char *current_path)
{
    printf("%s\n", current_path);
}

void copy_file(struct iso_dir *entry, void *iso_data)
{
    uint32_t file_extent = entry->extent.le;
    uint32_t file_size = entry->size.le;
    void *file_data = (char *)iso_data + file_extent * ISO_BLOCK_SIZE;
    char file_name[MAX_PATH_LEN];
    snprintf(file_name, entry->name_len + 1, "%s", entry->name);
    char *semicolon = strchr(file_name, ';');
    if (semicolon)
    {
        *semicolon = '\0';
    }

    FILE *output_file = fopen(file_name, "wb");
    if (!output_file)
    {
        perror("fopen");
        return;
    }
    fwrite(file_data, 1, file_size, output_file);
    fclose(output_file);
    printf("File %s copied successfully.\n", file_name);
}

void get(struct iso_dir *dir, const char *path, void *iso_data)
{
    uint32_t extent = dir->extent.le;
    uint32_t size = dir->size.le;
    void *dir_data = (char *)iso_data + extent * ISO_BLOCK_SIZE;
    size_t offset = 0;

    while (offset < size)
    {
        struct iso_dir *entry = (struct iso_dir *)((char *)dir_data + offset);
        if (entry->length == 0)
        {
            break;
        }
        char name[MAX_PATH_LEN];
        snprintf(name, entry->name_len + 1, "%s", entry->name);
        char *semicolon = strchr(name, ';');
        if (semicolon)
        {
            *semicolon = '\0';
        }
        if (strcmp(name, path) == 0)
        {
            if (entry->flags & ISO_FILE_ISDIR)
            {
                fprintf(stderr, "Error: %s is a directory\n", path);
                return;
            }
            copy_file(entry, iso_data);
            return;
        }
        offset += entry->length;
    }
    fprintf(stderr, "File not found: %s\n", path);
}

void cat(struct iso_dir *dir, const char *path, void *iso_data)
{
    uint32_t extent = dir->extent.le;
    uint32_t size = dir->size.le;
    void *dir_data = (char *)iso_data + extent * ISO_BLOCK_SIZE;
    size_t offset = 0;

    while (offset < size)
    {
        struct iso_dir *entry = (struct iso_dir *)((char *)dir_data + offset);
        if (entry->length == 0)
        {
            break;
        }
        char name[MAX_PATH_LEN];
        snprintf(name, entry->name_len + 1, "%s", entry->name);
        char *semicolon = strchr(name, ';');
        if (semicolon)
        {
            *semicolon = '\0';
        }
        if (strcmp(name, path) == 0)
        {
            if (entry->flags & ISO_FILE_ISDIR)
            {
                fprintf(stderr, "Error: %s is a directory\n", path);
                return;
            }
            uint32_t file_extent = entry->extent.le;
            uint32_t file_size = entry->size.le;
            void *file_data = (char *)iso_data + file_extent * ISO_BLOCK_SIZE;
            fwrite(file_data, 1, file_size, stdout);
            return;
        }
        offset += entry->length;
    }
    fprintf(stderr, "File not found: %s\n", path);
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <iso_file>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *iso_path = argv[1];
    int fd = open(iso_path, O_RDONLY);
    if (fd < 0)
    {
        perror("open");
        return EXIT_FAILURE;
    }

    struct stat st;
    if (fstat(fd, &st) < 0)
    {
        perror("fstat");
        close(fd);
        return EXIT_FAILURE;
    }

    void *iso_data = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (iso_data == MAP_FAILED)
    {
        perror("mmap");
        close(fd);
        return EXIT_FAILURE;
    }

    struct iso_prim_voldesc *vd =
        (struct iso_prim_voldesc *)((char *)iso_data + 0x8000);
    if (strncmp(vd->std_identifier, ISO_STANDARD_ID, ISO_STANDARD_ID_LEN) != 0)
    {
        fprintf(stderr, "Invalid ISO file\n");
        munmap(iso_data, st.st_size);
        close(fd);
        return EXIT_FAILURE;
    }

    struct iso_dir *current_dir = &vd->root_dir;
    struct iso_dir *root_dir = &vd->root_dir;
    char current_path[MAX_PATH_LEN] = "/";

    char command[4096];
    while (true)
    {
        if (isatty(STDIN_FILENO))
        {
            printf("\nisohelper: > ");
        }
        if (!fgets(command, sizeof(command), stdin))
        {
            break;
        }

        command[strcspn(command, "\n")] = '\0';

        if (strncmp(command, "help", 4) == 0)
        {
            print_help();
        }
        else if (strncmp(command, "info", 4) == 0)
        {
            info(vd);
        }
        else if (strncmp(command, "ls", 2) == 0)
        {
            ls(current_dir, iso_data);
        }
        else if (strncmp(command, "cd", 2) == 0)
        {
            char *path = command + 3;
            cd(&current_dir, path, current_path, root_dir, iso_data);
        }
        else if (strncmp(command, "get", 3) == 0)
        {
            char *path = command + 4;
            get(current_dir, path, iso_data);
        }
        else if (strncmp(command, "cat", 3) == 0)
        {
            char *path = command + 4;
            cat(current_dir, path, iso_data);
        }
        else if (strncmp(command, "pwd", 3) == 0)
        {
            pwd(current_path);
        }
        else if (strncmp(command, "quit", 4) == 0)
        {
            break;
        }
        else
        {
            fprintf(stderr, "Unknown command: %s\n", command);
        }
    }

    munmap(iso_data, st.st_size);
    close(fd);
    return EXIT_SUCCESS;
}
