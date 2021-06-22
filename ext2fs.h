#pragma once

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <sys/types.h>
#include "ext2.h"

#define SECTOR_SIZE 1024 //the sector size we use for testing purposes

typedef _Bool BOOL;

typedef struct ext2_fs_data {
    struct ext2_super_block* superblock;
    struct ext2_group_desc* group_descriptors;
    struct ext2_inode* root_inode;
    
    size_t block_group_desc_count;
} ext2_fs_data_t;

ext2_fs_data_t* ext2fs_mount(char* filename);
void ext2fs_unmount(ext2_fs_data_t* fs_data);

struct ext2_inode get_inode_from_num(ext2_fs_data_t* fs_data, uint32_t inodeIndex);
struct ext2_inode* get_file_inode(ext2_fs_data_t* fs_data, char* pathname);
uint32_t search_directory(ext2_fs_data_t* fs_data, struct ext2_inode* directory, char* filename);

void read_inode(ext2_fs_data_t* fs_data, struct ext2_inode* inode, void* buffer, size_t num_bytes);
void read_device(uint8_t* buffer, size_t lba, size_t len, size_t byte_offset);

static inline size_t get_block_size(ext2_fs_data_t* fs_data) {
    if (fs_data->superblock->s_magic == EXT2_MAGIC) {
        return 1024 << fs_data->superblock->s_log_block_size;
    } else {
        return 1024;
    }
}

static inline size_t num_blocks_to_read(ext2_fs_data_t* fs_data, size_t num_bytes) {
    if (num_bytes == 0) {
        return 1;
    }
    if (num_bytes % get_block_size(fs_data) != 0) {
        if (num_bytes < get_block_size(fs_data)) {
            return 1;
        }
        return (num_bytes / get_block_size(fs_data)) + 1;
    }
    return num_bytes / get_block_size(fs_data);
}