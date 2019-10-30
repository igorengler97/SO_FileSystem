#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <iostream>
#include <stdlib.h>
#include <math.h>

#include "superblock.h"
#include "blockgroup_descriptor.h"
#include "inode_bitmap.h"
#include "inode.h"
#include "directoryentry.h"

typedef struct filesystem {
    
    superblock* sb;
    blockgroup_descriptor* bg_d;

    unsigned int partition_size;
    
    filesystem();
    filesystem(int sectors);
    void format(FILE* file, int sectors);
    uint32_t getBlockSize();

} filesystem;

filesystem::filesystem(){

}

filesystem::filesystem(int sectors){
    this->partition_size = sectors * 512;
    this->sb  = new superblock(this->partition_size);
    //this->bg_d = new blockgroup_descriptor;
}

void filesystem::format(FILE* device, int sectors) {
    superblock sb_format;
    blockgroup_descriptor bg_d_format(sizeof(sb_format));
    inode_bitmap ib_format;
    inode i_format;

    sb_format.s_blocks_count = ceil((sectors*512)/sb_format.s_block_size);
    sb_format.s_inodes_count = ceil((sectors*512)/sb_format.s_block_size);
    sb_format.s_free_blocks_count = sb_format.s_blocks_count;
    sb_format.s_free_inodes_count = sb_format.s_inodes_count;

    sb_format.printSuperblock();

    rewind(device);
    sb_format.writeFile(device);
    bg_d_format.writeFile(device);
    ib_format.writeFile(device);

    for(int i = 0; i < sb_format.s_inodes_count; i++){
        i_format.writeFile(device);
    }
}

uint32_t filesystem::getBlockSize(){
    return sb->s_block_size;
}

#endif // FILESYSTEM_H