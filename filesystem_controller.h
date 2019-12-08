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
    void listDirectory(FILE* device, uint32_t entry_inode, char* op);
    void makedir(FILE* device, std::string token, uint32_t inode);
    void makefile(FILE* device, std::string name, uint32_t entry_inode, uint32_t entry_offset, FILE* new_file);
    void rmv(FILE* device, uint32_t inode);
    uint32_t findDentryDir(FILE* device, std::string name, uint32_t inode);
    std::pair<uint32_t, uint32_t> findDentryFile(FILE* device, std::string name_file, uint32_t dir_inode);
    uint32_t seekFreeInode(FILE* device);
    void writeInodeBitmap(FILE* device, uint32_t index_inode);
    void copy_file_HDtoFS(FILE* device, uint32_t dir_inode, std::string name_file, FILE* new_file);
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

    //sb_format.printSuperblock();

    rewind(device);
    sb_format.writeFile(device);
    bg_d_format.writeFile(device);
    ib_format.writeFile(device);

    for(int i = 0; i < sb_format.s_inodes_count; i++){
        i_format.writeFile(device);
    }

    //std::cout<<bg_d_format.bgd_addr_first_free_block << std::endl;

    // Insere na lista os blocos livres
    // 266753552
    for(int i = 0; i < (((sectors*512)-bg_d_format.bgd_data_blocks)/sb_format.s_block_size); i++) {
        if(i == 0) {
            free_blocks_list.push_back(bg_d_format.bgd_addr_first_free_block);
        } else {
            free_blocks_list.push_back(bg_d_format.bgd_addr_first_free_block + (i * 1024));
        }
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

void filesystem::listDirectory(FILE* device, uint32_t entry_inode, char* op){
    //Posiciona no arquivo o local onde esta as Entradas para o inode
    uint32_t inode_position;

    fseek(device, (this->bg_d->bgd_inode_table + (entry_inode * 64) + 5), SEEK_SET);
    fread(&inode_position, sizeof(uint32_t), 1, device);
    fseek(device, inode_position, SEEK_SET);

    for(int index_entry = 0; index_entry < this->sb->s_block_size; index_entry+=32){
        dentry entry;

        fread(&entry, sizeof(dentry), 1, device);

        if(entry.file_name[0] == 0){
            std::cout << std::endl;
            return;
        }else if (entry.file_type == 0x02){
            printf("\x1b[32m %25s \x1b[0m", entry.file_name); //diretorio verde
        }else if(entry.file_type == 0x01){
            printf("\x1b[33m %25s \x1b[0m", entry.file_name); // arquivo amarelo
        }
    }
    std::cout << std::endl;

    
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

void filesystem::makefile(FILE* device, std::string name, uint32_t entry_inode, uint32_t entry_offset, FILE* new_file){

}

void filesystem::rmv(FILE* device, uint32_t entry_inode) {
    // 1 - Cria uma lista/pilha e coloca o Número do INODE do Diretório/Arquivo a ser Excluido
    // 2 - Procura na Inode Table a informação referente ao Tipo de Entrada do Inode existente na LISTA DE INODES
    //      2.1 - Se for Arquivo apenas continua para o proximo inode da LISTA
    //      2.2 - Senão se for Diretório, ir até os Ponteiros de Data Block, e ler as Entradas de Cada DATA BLOCK daquele Diretorio
    // 3 - Para cada entrada do diretório, colocar na pilha os inodes referentes a cada entrada;
    // 4 - Anda na lista, para o próximo Inode, repete o processo até que se chegue ao fim da lista, ou seja, nao tenha mais entradas para ler (DEEP SEARCH)
    // 5 - Nesse estado teremos uma pilha com todos os INODES, a partir disso começar a desempilhar
    // 6 - Para cada inode, deve-se:
    //      6.1 - Escrever 0xE5 nas entradas dos Data Blocks, para delimitar entrada/arquivo excluido;
    //      6.2 - Adicionar a Lista de Blocos Livres os Blocos Apontados pelo Inode;
    //      6.3 - Atualizar Inode_Bitmap com os inodes agora livres;
    //      6.4 - Zerar Inodes

    uint8_t inode_type;
    uint32_t i_block_position[OWNFS_I_BLOCK_POINTERS];

    // 1
    std::vector<uint32_t> deep_search_vector;
    deep_search_vector.push_back(entry_inode);

    for (int i = 0; i < deep_search_vector.size(); i++){
        std::cout << deep_search_vector.size() << " - ";
        inode current_inode;
        uint32_t current_inode_number = deep_search_vector.at(i);

        std::cout << "CURRENT_INODE: " << current_inode_number << std::endl;

        // 2
        fseek(device, (this->bg_d->bgd_inode_table + (current_inode_number * 64)), SEEK_SET);
        fread(&current_inode, sizeof(inode), 1, device);
        
        // 2.1
        if(current_inode.i_type == 0x01){
            std::cout << "É arquivo, portanto é só continuar, para depois excluir utilizando o vector" << std::endl;
            continue;
        
        //2.2
        }else if(current_inode.i_type == 0x02){
            std::cout << "É diretorio, portanto precisa-se salvar todas as entradas dele" << std::endl;
            // 12 ponteiros direto para BLOCOS, portanto deve-se conferir todos
            for(int block_pointer = 0; block_pointer < 12; block_pointer++){
                // Se for "NULL" é porque nao esta sendo usado, então para
                if(current_inode.i_block[block_pointer] == 0x0000){
                    std::cout << " 0 - PONTEIRO PARA O BLOCO: " << current_inode.i_block[block_pointer] << " ";
                    break;
                }

                // Pular "." e ".."
                fseek(device, current_inode.i_block[block_pointer] + 64, SEEK_SET);
                
                // Ler entrada por entrada, jogando os INODES dela no VECTOR
                for(int index_entry = 64; index_entry < this->sb->s_block_size; index_entry+=32) {
                    dentry entry;
                    fread(&entry, sizeof(dentry), 1, device);
                    std::cout << "PONTEIRO PARA O BLOCO: " << current_inode.i_block[block_pointer] << std::endl;
                    std::cout << "ENTRY_INODE: " << entry.inode << std::endl;
                    //Se inode = 0 é porque não possui entrada
                    if(entry.inode == 0x0000){
                        break;
                    }else{
                        std::cout << "TESTE 1 - INODES FROM DIRECTORY: " << entry.inode << " ";
                        deep_search_vector.push_back(entry.inode);
                    }
                }
                std::cout << std::endl;
            }

            std::cout<<std::endl;
            
        }else{
            std::cout << "Inexistente, Corrompido ou Excluido" << std::endl;
        }
    }

    for (auto i = deep_search_vector.begin(); i != deep_search_vector.end(); ++i)
        std::cout << *i << " ";

}

uint32_t filesystem::findDentryDir(FILE* device, std::string name, uint32_t inode) {
    if(name.size() > 24)
        std::cout << "ERROR: Name greater than " << name.size() + 1 << " characters!" << std::endl;

    uint32_t inode_position;

    //Posiciona no arquivo o local onde esta as Entradas para o inode
    fseek(device, (this->bg_d->bgd_inode_table + (inode * 64) + 5), SEEK_SET);
    fread(&inode_position, sizeof(uint32_t), 1, device);
    fseek(device, inode_position, SEEK_SET);

    for(int index_entry = 0; index_entry < this->sb->s_block_size; index_entry+=32) {
        dentry entry;
        
        fread(&entry, sizeof(dentry), 1, device);

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

std::pair<uint32_t, uint32_t> filesystem::findDentryFile(FILE* device, std::string name_file, uint32_t dir_inode){
    if(name_file.size() > 24)
        std::cout << "ERROR: Name greater than " << name_file.size() + 1 << " characters!" << std::endl;

    uint32_t inode_position;

    //Posiciona no arquivo o local onde esta as Entradas para o ROOT
    fseek(device, (this->bg_d->bgd_inode_table + (dir_inode * 64) + 5), SEEK_SET);
    fread(&inode_position, sizeof(uint32_t), 1, device);
    fseek(device, inode_position, SEEK_SET);

    //std::cout << "inode_position " << inode_position << std::endl;

    for(int index_entry = 0; index_entry < this->sb->s_block_size; index_entry+=32) {
        dentry entry;
        
        // Procurar nas entradas do ROOT um Diretorio de mesmo nome
        fread(&entry, sizeof(dentry), 1, device);

        std::pair<uint32_t, uint32_t> info;

        int i = 0, flag = 0;
        for(i = 0; (i < name_file.size() && i < 24); i++) {
            if(entry.file_name[i] != name_file[i]) {
                flag = 1;
                break;
            }
        }

        int j = 0;
        for(j = 0; (j < name_file.size() && i < 24); i++) {
            if(entry.file_name[i] != '0') {
                flag = -1;
                break;
            }
        }

        if(!flag && (i == 24 || entry.file_name[i] == 0) && entry.file_type == 1) {
            std::cout << "File already exists!" << std::endl;
            exit(-1);
        } else {
            continue;
        }
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
    return -1;
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

void filesystem::copy_file_HDtoFS(FILE* device, uint32_t dir_inode, std::string name_file, FILE* new_file){
    dentry dentry;
    uint32_t inode_position;

    //Posiciona no arquivo o local onde esta as Entradas para diretorio de entrada
    fseek(device, (this->bg_d->bgd_inode_table + (dir_inode * 64) + 5), SEEK_SET);
    fread(&inode_position, sizeof(uint32_t), 1, device);
    fseek(device, inode_position, SEEK_SET);




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