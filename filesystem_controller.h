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
    int tam_partition;

    superblock* s;
    blockgroup_descriptor* bg_d;
    //blockgroup* bg[(tam_partition / ((s.blocksize * 8) * blocksize))];
    
    filesystem();
    filesystem(int tam_partition);

    void format(FILE* file, int sectors);

    uint32_t getBlockSize();

} filesystem;

filesystem::filesystem(){

}

filesystem::filesystem(int sectors){
    this->tam_partition = sectors * 512;
    this->s  = new superblock(this->tam_partition);
    this->bg_d = new blockgroup_descriptor;
    //this->bg  = new blockgroup(*s, *bg_d);
}

void filesystem::format(FILE* file, int sectors) {
    superblock s_format;
    s_format.s_blocks_count = sectors;

    std::cout << "How many sectors? " << s_format.s_blocks_count << std::endl;
}

uint32_t filesystem::getBlockSize(){
    return s->blocksize;
}

#endif // FILESYSTEM_H