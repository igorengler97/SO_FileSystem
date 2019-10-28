#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <iostream>
#include <stdlib.h>
#include <math.h>

#include "blockgroup.h"

typedef struct filesystem {
    superblock* s;
    blockgroup_descriptor* bg_d;
    blockgroup* bg;
    
    filesystem();
    filesystem(int sectors);

    void format(FILE* file, int sectors);

    uint32_t getBlockSize();

} filesystem;

filesystem::filesystem(){

}

filesystem::filesystem(int sectors){
    this->s  = new superblock(sectors);
    this->bg_d = new blockgroup_descriptor;
    this->bg  = new blockgroup(*s, *bg_d);
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