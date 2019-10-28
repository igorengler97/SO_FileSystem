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

blockgroup::blockgroup() {
    this->s    = new superblock;
    this->bg_d = new blockgroup_descriptor;
    this->i    = new inode;
    this->d    = new dentry;
}

#endif // BLOCKGROUP_H