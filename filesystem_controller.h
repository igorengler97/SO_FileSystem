#include <iostream>
#include <stdlib.h>
#include <math.h>

#include "superblock.h"
#include "blockgroup_descriptor.h"
#include "inode.h"
#include "directoryentry.h"

typedef struct filesystem {
    superblock* s;
    blockgroup_descriptor* bg_d;
    inode* i;
    dentry* d;

    filesystem();

    superblock getSuperblock() {
        //return s;
    }

    uint32_t getBlocksize() {
        //return s.blocksize;
    }

} filesystem;

filesystem::filesystem(){
    this->s = new superblock;
    this->bg_d = new blockgroup_descriptor;
    this->i = new inode;
    this->d = new dentry;
}