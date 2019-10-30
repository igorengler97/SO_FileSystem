#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <iostream>
#include <stdlib.h>
#include <math.h>

#include "superblock.h"
#include "blockgroup_descriptor.h"
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

void filesystem::format(FILE* file, int sectors) {
    superblock sb_format;
    sb_format.s_blocks_count = sectors*512;
    sb_format.s_inodes_count = sectors*512;
    sb_format.s_free_blocks_count = sb_format.s_blocks_count;
    sb_format.s_free_inodes_count = sb_format.s_inodes_count;

    sb_format.printSuperblock();
    
    
   
}

uint32_t filesystem::getBlockSize(){
    return sb->getBlockSize();
}

#endif // FILESYSTEM_H