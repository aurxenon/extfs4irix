#include "ext2fs.h"

int main(int argc, char** argv) {
    if (argc < 2) {
        printf("Error: needs a filesystem to mount\n");
        return -1;
    }
    if (argc < 3) {
        printf("Error: needs a file to read\n");
        return -1;
    }

    printf("Mounting filesystem located at %s\n", argv[1]);
    ext2_fs_data_t* fs_data = ext2fs_mount(argv[1]);

    if (fs_data == NULL) {
        printf("Error: unable to mount filesystem\n");
        return -1;
    }

    struct ext2_inode* file = get_file_inode(fs_data, argv[2]);
    if (file == NULL) {
        ext2fs_unmount(fs_data);
        return -1;
    }
    char* buffer = malloc(le32toh(file->i_size) + 1);
    buffer[le32toh(file->i_size)] = 0;
    read_inode(fs_data, file, buffer, le32toh(file->i_size));
    
    printf("%s contents: %s\n", argv[2], buffer);
    free(buffer);
    free(file);

    ext2fs_unmount(fs_data);

    return 0;
}