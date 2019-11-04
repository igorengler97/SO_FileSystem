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
    dentry* d;

    std::vector<uint32_t> free_blocks_list;

    unsigned int partition_size;
    
    filesystem();
    filesystem(int sectors);
    void format(FILE* device, int sectors);
    void mount(FILE* device);
    void makedir(FILE* device, std::string token, uint32_t inode);
    uint32_t findDentryDir(FILE* device, std::string name, uint32_t inode);
    std::pair<uint32_t, uint32_t> findDentryFile(FILE* device, std::string name, std::string extension, uint32_t inode_entry);
    uint32_t seekFreeInode(FILE* device);
    void writeInodeBitmap(FILE* device, uint32_t index_inode);
    void copy_file_HDtoFS(FILE* file, uint32_t dir_inode, uint32_t inode_entry, uint32_t entry_offset);
    uint32_t getBlockSize();
    
    void updateFreeBlockList(FILE* device, uint32_t *x);

} filesystem;

filesystem::filesystem(){
    this->sb = new superblock;
    this->bg_d = new blockgroup_descriptor;
    this->d = new dentry;
}

/*
filesystem::filesystem(int sectors){
    this->partition_size = sectors * 512;
    this->sb  = new superblock(this->partition_size);
    //this->bg_d = new blockgroup_descriptor;
}
*/

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
        std::fseek(device, free_blocks_list.at(i), SEEK_SET);
        std::fwrite(x+(i+1), sizeof(uint32_t), 1, device);
        if(i == (free_blocks_list.size() - 1)){
            std::fseek(device, free_blocks_list.at(i)+1020, SEEK_SET);
            std::fwrite(x+(i+1), sizeof(uint32_t), 1, device);
        }
    }

    i_format.i_type = 2;
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

    uint8_t byte = 1 << 7;
    
    std:fseek(device, bg_d_format.bgd_inode_bitmap, SEEK_SET);
    std::fwrite(&byte, sizeof(uint8_t), 1, device);

    //Tira da lista o Bloco utilizado, altera na imagem a lista de blocos livres
    free_blocks_list.erase(free_blocks_list.begin());
    x = free_blocks_list.data();
    bg_d_format.bgd_addr_first_free_block = free_blocks_list[0];
    updateFreeBlockList(device, x);
}

void filesystem::mount(FILE* device){
    fread(sb, sizeof(superblock), 1, device);
    fread(bg_d, sizeof(blockgroup_descriptor), 1, device);
    
    uint32_t free_block;

    free_blocks_list.push_back(bg_d->bgd_addr_first_free_block);
    fseek(device, bg_d->bgd_addr_first_free_block, SEEK_SET);
    
    while(free_block != 0x00000000) {
        fread(&free_block, sizeof(uint32_t), 1, device);
        free_blocks_list.push_back(free_block);
        fseek(device, free_block, SEEK_SET);
    }
}

void filesystem::makedir(FILE* device, std::string name, uint32_t entry_inode) {
    // Encontrar um Inode livre no InodeBitmap
    uint32_t free_inode = seekFreeInode(device);

    // Ir na InodeTable e escrever o respectivo Inode
    fseek(device, bg_d->bgd_inode_table + (free_inode * 64), SEEK_SET);
    inode new_inode;
    new_inode.i_type = 2;
    new_inode.i_size = 0;
    new_inode.i_block[0] = free_blocks_list[0];
    fwrite(&new_inode, sizeof(inode), 1, device);
    
    // Apagar o ex Bloco livre da lista de blocos livres (free_list_blocks)
    uint32_t *x = free_blocks_list.data();
    free_blocks_list.erase(free_blocks_list.begin());
    x = free_blocks_list.data();
    bg_d->bgd_addr_first_free_block = free_blocks_list[0];
    updateFreeBlockList(device, x);

    // Configurando as entradas
    // Escreve no Root
    dentry new_entry[3];
    new_entry[0].inode = free_inode;
    new_entry[0].entry_len = sizeof(dentry);
    new_entry[0].name_len = sizeof(new_entry[0].file_name);
    new_entry[0].file_type = 2;
    new_entry[0].setName(name);

    // Escreve no Bloco do i_block do novo dir {
    new_entry[1].inode = free_inode;
    new_entry[1].entry_len = sizeof(dentry);
    new_entry[1].name_len = sizeof(new_entry[0].file_name);
    new_entry[1].file_type = 2;
    new_entry[1].setName(".");

    new_entry[2].inode = entry_inode;
    new_entry[2].entry_len = sizeof(dentry);
    new_entry[2].name_len = sizeof(new_entry[0].file_name);
    new_entry[2].file_type = 2;
    new_entry[2].setName("..");
    // }

    std::fseek(device, new_inode.i_block[0], SEEK_SET);
    fwrite(&new_entry[1], sizeof(dentry), 1, device);
    fwrite(&new_entry[2], sizeof(dentry), 1, device);
    
    fseek(device, bg_d->bgd_inode_table + (entry_inode * 64) + 5, SEEK_SET);
    uint32_t addr_entry;
    fread(&addr_entry, sizeof(uint32_t), 1, device);
    fseek(device, addr_entry, SEEK_SET);

    for(int index_entry = 0; index_entry < this->sb->s_block_size; index_entry+=32) {
        dentry entry;
        
        // Procurar nas entradas do ROOT um Diretorio de mesmo nome
        fread(&entry, sizeof(dentry), 1, device);
        int flag = 0, j = 0;
        for(j = 0; j < 24; j++) {
            if(entry.file_name[j] != 0) {
                flag = 1;
                break;
            }
        }

        if(flag == 0 && j == 24){
            addr_entry += index_entry;
            break;
        }
    }

    fseek(device, addr_entry, SEEK_SET);
    fwrite(&new_entry[0], sizeof(dentry), 1, device);
    writeInodeBitmap(device, free_inode);
}

uint32_t filesystem::findDentryDir(FILE* device, std::string name, uint32_t inode) {
    if(name.size() > 24)
        std::cout << "ERROR: Name greater than " << name.size() + 1 << " characters!" << std::endl;

    uint32_t inode_position;

    //Posiciona no arquivo o local onde esta as Entradas para o ROOT
    fseek(device, (this->bg_d->bgd_inode_table + (inode * 64) + 5), SEEK_SET);
    fread(&inode_position, sizeof(uint32_t), 1, device);
    fseek(device, inode_position, SEEK_SET);

    for(int index_entry = 0; index_entry < this->sb->s_block_size; index_entry+=32) {
        dentry entry;
        
        // Procurar nas entradas do ROOT um Diretorio de mesmo nome
        fread(&entry, sizeof(dentry), 1, device);

        //std::cout << "inode: " << entry.inode << std::endl;
        //std::cout << "entry len: " << entry.entry_len << std::endl;
        //std::cout << "name len: " << unsigned(entry.name_len) << std::endl;
        //std::cout << "file type: " << unsigned(entry.file_type) << std::endl;
        //std::cout << "file name: " << entry.file_name << std::endl;

        int j = 0, flag = 0;
        for(j = 0; (j < name.size() && j < 24); j++) {
            if(entry.file_name[j] != name[j]) {
                flag = 1;
                break;
            }
        }

        if(!flag && (j == 24 || entry.file_name[j] == 0) && entry.file_type == 2) {
            return entry.inode;
        }
    }
    return -1;
}

std::pair<uint32_t, uint32_t> filesystem::findDentryFile(FILE* device, std::string name, std::string extension, uint32_t inode_entry){
    if(name.size() > 24)
        std::cout << "ERROR: Name greater than " << name.size() + 1 << " characters!" << std::endl;

    uint32_t inode_position;

    //Posiciona no arquivo o local onde esta as Entradas para o ROOT
    fseek(device, (this->bg_d->bgd_inode_table + (inode_entry * 64) + 5), SEEK_SET);
    fread(&inode_position, sizeof(uint32_t), 1, device);
    fseek(device, inode_position, SEEK_SET);

    for(int index_entry = 0; index_entry < this->sb->s_block_size; index_entry+=32) {
        dentry entry;
        
        // Procurar nas entradas do ROOT um Diretorio de mesmo nome
        fread(&entry, sizeof(dentry), 1, device);

        //std::cout << "inode: " << entry.inode << std::endl;
        //std::cout << "entry len: " << entry.entry_len << std::endl;
        //std::cout << "name len: " << unsigned(entry.name_len) << std::endl;
        //std::cout << "file type: " << unsigned(entry.file_type) << std::endl;
        //std::cout << "file name: " << entry.file_name << std::endl;

        std::pair<uint32_t, uint32_t> info;

        // FILE_NAME {
        int i = 0, flag = 0;
        for(i = 0; (i < name.size() && i < 24); i++) {
            if(entry.file_name[i] != name[i]) {
                flag = 1;
                break;
            }
        }

        if(!flag && (i == 24 || entry.file_name[i] == 0) && entry.file_type == 1) {
            info.first = inode_entry;
            info.second = index_entry;
        } else {
            continue;
        }
        // }

        // EXTENSION {
        flag = 0, i = 0;
		for( i = 0; i < extension.size() && i < 3; i++ ){
			if( Rt.extension[i] != extension[i] ){
				flag = 1;
				break;
			}
            if(entry.[i] != name[i]) {
                flag = 1;
                break;
            }
		}

		if( !flag && ( i == 3 || Rt.extension[i] == 0) && Rt.attribute_file == 0x20){
			return ans;
		}else{
			continue;
		}
        // }

    }
    return std::pair<uint32_t, uint32_t>(-1, -1);
}

uint32_t filesystem::seekFreeInode(FILE* device) {
    uint8_t mask = 0xFF;
    uint8_t byte = 0x00;

    fseek(device, bg_d->bgd_inode_bitmap, SEEK_SET);

    for(uint32_t i = bg_d->bgd_inode_bitmap; i < sizeof(inode_bitmap); i++) {
        fread(&byte, sizeof(uint8_t), 1, device);

        uint8_t free = (byte ^ mask);

        if(!free) {
            continue;
        }

        for(uint8_t s = 0; s < 8; s++) {
            if((free << s) & 0x80) {
                return ((s + 1) + (8 * (i - bg_d->bgd_inode_bitmap)))-1;
            }
        }
    }
}

void filesystem::writeInodeBitmap(FILE* device, uint32_t index_inode) {
    uint32_t offset = index_inode / 8;
    uint32_t shift  = index_inode % 8;
    uint8_t  mask   = 0x80;
    uint8_t  entry  = 0x00;

    //std::cout << "offset " << offset << std::endl;
    //std::cout << "shift  " << shift  << std::endl;

    fseek(device, bg_d->bgd_inode_bitmap + offset, SEEK_SET);
    fread(&entry, sizeof(uint8_t), 1, device);
    entry |= mask >> shift;
    fseek(device, bg_d->bgd_inode_bitmap + offset, SEEK_SET);
    fwrite(&entry, sizeof(uint8_t), 1, device);
}

void filesystem::copy_file_HDtoFS(FILE* file, uint32_t dir_inode, uint32_t inode_entry, uint32_t entry_offset){
    dentry dentry;

    fseek(device, );
}

uint32_t filesystem::getBlockSize(){
    return sb->s_block_size;
}

void filesystem::updateFreeBlockList(FILE* device, uint32_t *x){
     
    // Altera o primeiro Bloco livre no Block Groups Descriptor
    std::fseek(device, sizeof(superblock) + 12, SEEK_SET );
    std::fwrite(x, sizeof(uint32_t), 1, device);

    //Reescreve na imagem a lista ligada de blocos livres
    for(int i = 0; i < free_blocks_list.size() - 1; i++){
        std::fseek(device, free_blocks_list.at(i), SEEK_SET);
        std::fwrite(x+(i+1), sizeof(uint32_t), 1, device);
    }
}

#endif // FILESYSTEM