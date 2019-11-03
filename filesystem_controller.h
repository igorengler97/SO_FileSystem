#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <iostream>
#include <vector>
#include <algorithm>
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
    std::vector<uint32_t> free_blocks_list;

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
    dentry de_format[2];

    sb_format.s_blocks_count = ceil((double)((sectors*512)/sb_format.s_block_size));
    sb_format.s_inodes_count = ceil((double)((sectors*512)/sb_format.s_block_size)/10);
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

    std::cout<<bg_d_format.bgd_addr_first_free_block << std::endl;

    // Insere na lista os blocos livres
    // 266753552
    for(int i = 0; i < (((sectors*512)-bg_d_format.bgd_data_blocks)/sb_format.s_block_size); i++) {
        if(i == 0) {
            free_blocks_list.push_back(bg_d_format.bgd_addr_first_free_block);
        } else {
            free_blocks_list.push_back(bg_d_format.bgd_addr_first_free_block + (i * 1024));
        }
        std::cout << free_blocks_list.at(i) << std::endl;
    }

    uint32_t *x = free_blocks_list.data();
    
    for(int i = 0; i < free_blocks_list.size(); i++){
        fseek(device, free_blocks_list.at(i), SEEK_SET);
        fwrite(x+(i+1), sizeof(uint32_t), 1, device);
        if(i == (free_blocks_list.size() - 1)){
            fseek(device, free_blocks_list.at(i)+1020, SEEK_SET);
            fwrite(x+(i+1), sizeof(uint32_t), 1, device);
        }
    }

    i_format.i_type = 0x4000;
    i_format.i_block[0] = bg_d_format.bgd_data_blocks;
    i_format.i_size = 0;
    std::fseek(device, bg_d_format.bgd_inode_table, SEEK_SET);
    std::fwrite(&i_format, sizeof(inode), 1, device);

    de_format[0].inode = 0;
    de_format[0].setName(".");
    de_format[0].name_len = sizeof(de_format[0].file_name);
    de_format[0].file_type = 2;
    de_format[0].entry_len = sizeof(de_format[0]);

    de_format[1].inode = 0;
    de_format[1].setName("..");
    de_format[1].name_len = sizeof(de_format[1].file_name);
    de_format[1].file_type = 2;
    de_format[1].entry_len = sizeof(de_format[1]);

    std::fseek(device, bg_d_format.bgd_data_blocks, SEEK_SET);
    std::fwrite(de_format, sizeof(dentry), 2, device);

    uint8_t byte = 1 >> 7;
    
    std:fseek(device, bg_d_format.bgd_inode_bitmap, SEEK_SET);
    std::fwrite(&byte, sizeof(uint8_t), 1, device);

    //Tira da lista o Bloco utilizado, altera na imagem a lista de blocos livres

}

uint32_t filesystem::getBlockSize(){
    return sb->s_block_size;
}

#endif // FILESYSTEM_H