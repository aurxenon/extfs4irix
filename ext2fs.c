#include "ext2fs.h"

char* fs_file;

ext2_fs_data_t* ext2fs_mount(char* filename) {
    fs_file = filename;
    ext2_fs_data_t* fs_data = (ext2_fs_data_t*)malloc(sizeof(ext2_fs_data_t));

    fs_data->superblock = (struct ext2_super_block*)malloc(sizeof(struct ext2_super_block));

    if (fs_data->superblock == NULL) {
        printf("Error: unable to allocate space for superblock\n");

        free(fs_data);
        return NULL;
    }

    read_device((uint8_t*)fs_data->superblock, 1, sizeof(struct ext2_super_block), 0);

    if (le16toh(fs_data->superblock->s_magic) != EXT2_MAGIC) {
        printf("Error: magic not detected\n");

        free(fs_data->superblock);
        free(fs_data);

        return NULL;
    }

    fs_data->block_group_desc_count = 
        le32toh(fs_data->superblock->s_blocks_count) / le32toh(fs_data->superblock->s_blocks_per_group);
    
    //if the number of descriptors is not an evenly divisible number, round the result up
    if (le32toh(fs_data->superblock->s_blocks_count) % le32toh(fs_data->superblock->s_blocks_per_group) != 0) { 
        fs_data->block_group_desc_count++;
    }

    size_t desc_bytes_occupied = fs_data->block_group_desc_count * sizeof(struct ext2_group_desc);
    size_t desc_blocks_occupied = num_blocks_to_read(fs_data, desc_bytes_occupied);

    fs_data->group_descriptors = (struct ext2_group_desc*)malloc(desc_bytes_occupied);

    if (get_block_size(fs_data) == 1024) {
        read_device((uint8_t*)fs_data->group_descriptors, 2, desc_bytes_occupied, 0);
    } else {
        printf("Error: filesystem block size not 1024\n");

        free(fs_data->superblock);
        free(fs_data->group_descriptors);
        free(fs_data);

        return NULL;
    }

    fs_data->root_inode = (struct ext2_inode*)malloc(sizeof(struct ext2_inode));
    *(fs_data->root_inode) = get_inode_from_num(fs_data, EXT2_ROOT_INO);
    return fs_data;
}

void ext2fs_unmount(ext2_fs_data_t* fs_data) {
    free(fs_data->superblock);
    free(fs_data->group_descriptors);
    free(fs_data->root_inode);
    free(fs_data);
}

struct ext2_inode get_inode_from_num(ext2_fs_data_t* fs_data, uint32_t inodeIndex) {
    //determine which block group contains the inode
    uint32_t block_group = (inodeIndex - 1) / le32toh(fs_data->superblock->s_inodes_per_group);
    //figure out what the index is within the block group
    uint32_t inode_index_in_block_group = (inodeIndex - 1) % le32toh(fs_data->superblock->s_inodes_per_group);
    //figure out how many bytes away it is
    uint32_t inode_byte_offset = inode_index_in_block_group * sizeof(struct ext2_inode);
    //figure out how many blocks away it is from the inode table start
    uint32_t containing_block = inode_byte_offset / get_block_size(fs_data); //rounds down
    //figure out byte offset within block
    uint32_t inode_offset_in_block = inode_byte_offset - (containing_block * get_block_size(fs_data));
    //figure out the block that's actually containing the inode
    uint32_t inode_block = le32toh(fs_data->group_descriptors[block_group].bg_inode_table) + containing_block;

    struct ext2_inode inode;

    read_device((uint8_t*)&inode, inode_block, sizeof(struct ext2_inode), inode_offset_in_block);

    return inode;
}

struct ext2_inode* get_file_inode(ext2_fs_data_t* fs_data, char* pathname) {
    char* pathname_copy = malloc(strlen(pathname) + 1);
    //we need pathname_copy_orig to store the pointer for free
    char* pathname_copy_orig = pathname_copy;
    strcpy(pathname_copy, pathname);
    struct ext2_inode search_inode = *(fs_data->root_inode);

    char* token = strsep(&pathname_copy, "/");
    
    //for now, we just assume every path is relative to the root directory
    while (token != NULL) {
        if (*token == '\0') {
            token = strsep(&pathname_copy, "/");
            continue;
        }
        uint32_t search_inode_num = search_directory(fs_data, &search_inode, token);
        if (!search_inode_num) {
            printf("Error: could not locate %s!\n", token);

            free(pathname_copy_orig);
            return NULL;
        }
        search_inode = get_inode_from_num(fs_data, search_inode_num);
        token = strsep(&pathname_copy, "/");
    }

    free(pathname_copy_orig);

    struct ext2_inode* inode = (struct ext2_inode*)malloc(sizeof(struct ext2_inode));
    *inode = search_inode;
    
    return inode;
}

uint32_t search_directory(ext2_fs_data_t* fs_data, struct ext2_inode* directory, char* filename) {
    size_t dir_search_size = le32toh(directory->i_size);
    size_t bytes_read = 0;

    //used for keeping track of starting position for free()
    struct ext2_dir_entry* dir_entries_start = (struct ext2_dir_entry*)malloc(dir_search_size);
    //this is what is used for iteration
    struct ext2_dir_entry* dir_entries = dir_entries_start;
    read_inode(fs_data, directory, dir_entries, dir_search_size);

    while (bytes_read <= dir_search_size) {
        if (strncmp(dir_entries->name, filename, dir_entries->name_len) == 0) {
            uint32_t inode_num = le32toh(dir_entries->inode);
            free(dir_entries_start);
            return inode_num;
        }
        bytes_read = bytes_read + le16toh(dir_entries->rec_len);
        //for some reason if i dont cast dirEntries to a size_t it doesn't add correctly
        dir_entries = (struct ext2_dir_entry*)((size_t)dir_entries + le16toh(dir_entries->rec_len));
    }

    free(dir_entries_start);
    return EXT2_INVALID_INODE;
}

void read_inode(ext2_fs_data_t* fs_data, struct ext2_inode* inode, void* buffer, size_t num_bytes) {
    void* buffer_copy = buffer;
    if (num_bytes <= (EXT2_NDIR_BLOCKS * get_block_size(fs_data))) {
        size_t block_count = num_blocks_to_read(fs_data, num_bytes);
        int bytes_left = num_bytes;

        for (size_t i = 0; i < block_count; i++) {
            if (bytes_left < get_block_size(fs_data)) {
                //reads in the remaining bytes (if the remaining bytes are less than a block)
                if (le32toh(inode->i_block[i]) != 0) {
                    read_device(buffer_copy, le32toh(inode->i_block[i]), bytes_left, 0);
                }
            } else {
                //reads in the remaining bytes (if block-sized)
                if (le32toh(inode->i_block[i]) != 0) {
                    read_device(buffer_copy, le32toh(inode->i_block[i]), get_block_size(fs_data), 0);
                    buffer_copy += get_block_size(fs_data);
                    bytes_left -= get_block_size(fs_data);
                }
            }
        }
    }
}

void read_device(uint8_t* buffer, size_t lba, size_t len, size_t byte_offset) {
    FILE* ext2FSFile = fopen(fs_file, "r");

    if (ext2FSFile == NULL) {
        printf("Error: unable to open %s\n", fs_file);
        return;
    }

    int c;
    fseek(ext2FSFile, (lba * SECTOR_SIZE) + byte_offset, SEEK_SET);

    for (uint32_t i = 0; i < len; i++) {
        c = fgetc(ext2FSFile);

        if (c == EOF) {
            fclose(ext2FSFile);
            return;
        }
        buffer[i] = (uint8_t)c;
    }

    fclose(ext2FSFile);
}