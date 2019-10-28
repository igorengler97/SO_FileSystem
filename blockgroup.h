#ifndef BLOCKGROUP_H
#define BLOCKGROUP_H

#include "superblock.h"
#include "blockgroup_descriptor.h"
#include "inode.h"
#include "directoryentry.h"

typedef struct blockgroup {
    superblock* s;
    blockgroup_descriptor* bg_d;
    inode* i;
    dentry* d;

    blockgroup();

} __attribute__((__packed__)) blockgroup;

blockgroup::blockgroup(superblock s, blockgroup_descriptor bg) {
    this->s    = s;
    this->bg_d = bg;
    this->i    = new inode;
    this->d    = new dentry;
}

#endif // BLOCKGROUP_H