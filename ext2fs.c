#include "ext2fs.h"

char* fs_file;

int main(int argc, char** argv) {
    if (argc < 1) {
        printf("Error: needs a filesystem to mount\n");
        return -1;
    }

    printf("Mounting filesystem located at %s\n", argv[1]);
    struct ext2_super_block* sb = ext2fs_mount(argv[1]);

    if (sb == NULL) {
        printf("Error: unable to mount filesystem\n");
        return -1;
    }

    printf("sb->s_inodes_count: %"PRIu32"\n", sb->s_magic);

    free(sb);

    return 0;
}

struct ext2_super_block* ext2fs_mount(char* filename) {
    fs_file = filename;
    struct ext2_super_block* sb = (struct ext2_super_block*)malloc(sizeof(struct ext2_super_block));

    if (sb == NULL) {
        printf("Error: Unable to allocate space for superblock\n");
        return NULL;
    }

    read_device(sb, 1, sizeof(struct ext2_super_block));

    if (sb->s_magic != EXT2_MAGIC) {
        printf("Error: magic not detected\n");
        return NULL;
    }

    return sb;
}

void read_device(uint8_t* buffer, size_t lba, size_t len) {
    FILE* ext2FSFile = fopen(fs_file, "r");

    if (ext2FSFile == NULL) {
        return;
    }

    int c;
    uint32_t counter;

    for (uint32_t i = 0; i < (lba * SECTOR_SIZE) + len; i++) {
        c = fgetc(ext2FSFile);

        if (c == EOF) {
            return;
        }
        if (i >= (lba * SECTOR_SIZE)) {
            buffer[i - (lba * SECTOR_SIZE)] = (uint8_t)c;
        }
    }
}