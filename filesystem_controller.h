#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <iostream>
#include <stdlib.h>
#include <math.h>

#include "blockgroup.h"

typedef struct filesystem {
    blockgroup *g;

    filesystem();

    void format(FILE* file, int sectors);

    superblock getSuperblock() {
        //return s;
    }

    uint32_t getBlocksize() {
        //return s.blocksize;
    }

} filesystem;

filesystem::filesystem(){
    this->g = new blockgroup;
}

void filesystem::format(FILE* file, int sectors) {
    superblock s_format;
    s_format.s_blocks_count = sectors;

    std::cout << "How many sectors? " << s_format.s_blocks_count << std::endl;
}

#endif // FILESYSTEM_H