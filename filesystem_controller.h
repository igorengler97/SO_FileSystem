#include <iostream>
#include <stdlib.h>
#include <math.h>

#include "superblock.h"
#include "blockgroup_descriptor.h"
#include "inode.h"
#include "directoryentry.h"

typedef struct filesystem {
    superblock s;
    blockgroup_descriptor bg;
    inode i;
    dentry d;

    filesystem() {     // look, a constructor
        
    }

    superblock getSuperblock() {
        return s;
    }

    uint32_t getBlocksize() {
        return s.blocksize;
    }

} filesystem;