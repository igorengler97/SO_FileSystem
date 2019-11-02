#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <iostream>
#include <list>
#include <cstddef>
#include <iterator>
#include <stdlib.h>
#include <math.h>

#include "superblock.h"
#include "blockgroup_descriptor.h"
#include "inode_bitmap.h"
#include "inode.h"
#include "directoryentry.h"
#include "block.h"

typedef struct filesystem {
    
    superblock* sb;
    blockgroup_descriptor* bg_d;
    block *free_blocks_list;

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

    // Insere na lista os blocos livres
    for(int i = 0; i < 10; i++) {
        if(i == 0) {
            free_blocks_list->data = bg_d_format.bgd_addr_first_free_block;
        } else {
            free_blocks_list.push_back(bg_d_format.bgd_addr_first_free_block + (i * 1024));
        }
    }

    /*
        Exemplo:
            // Be sure to have opened the file in binary mode
            Car *x = head;

            // Walk the list and write each node.
            // No need to write the next field - which happens to be the last one.
            //                    v-----------------v size of data before the `next` field
            while (x && fwrite(x, offsetof(Car, next), 1, out_stream) == 1) {
                x = x->next;
            }
    */

    // Grava na imagem os blocos
    for(int i = 0; i < 10; i++) {
        fwrite(free_blocks_list, sizeof(free_blocks_list), 1, device);
    }
}

uint32_t filesystem::getBlockSize(){
    return sb->s_block_size;
}

#endif // FILESYSTEM_H