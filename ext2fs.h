#pragma once

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <inttypes.h>
#include <sys/types.h>
#include "ext2.h"

#define SECTOR_SIZE 1024 //the sector size we use for testing purposes

typedef _Bool BOOL;

struct ext2_super_block* ext2fs_mount(char* filename);

void read_superblock(struct ext2_super_block* sb);
void read_device(uint8_t* buffer, size_t lba, size_t len);