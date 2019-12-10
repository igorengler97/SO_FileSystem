#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <iostream>
#include <vector>
#include <algorithm>
#include <stdlib.h>
#include <math.h>
#include <cstdio>
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
    uint32_t findDentryFile(FILE* device, std::string name, uint32_t inode);
    uint32_t seekFreeInode(FILE* device);
    void writeFreeInodeBitmap(FILE* device, uint32_t index_inode);
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

void filesystem::format(FILE* device, int sectors) {    // FORMATAR DISPOSITIVO
    superblock sb_format;
    blockgroup_descriptor bg_d_format(sizeof(sb_format));
    inode_bitmap ib_format;
    inode i_format;
    dentry de_format[2];

    sb_format.s_blocks_count = ceil((double)((sectors*512)/sb_format.s_block_size));
    sb_format.s_inodes_count = ceil((double)((sectors*512)/sb_format.s_block_size)/10);
    sb_format.s_free_blocks_count = sb_format.s_blocks_count;
    sb_format.s_free_inodes_count = sb_format.s_inodes_count;

    rewind(device);
    sb_format.writeFile(device);
    bg_d_format.writeFile(device);
    ib_format.writeFile(device);

    for(int i = 0; i < sb_format.s_inodes_count; i++){
        i_format.writeFile(device);
    }

    // Adiciona a lista (free_blocks_list) os BLOCOS LIVRES.
    for(int i = 0; i < (((sectors*512)-bg_d_format.bgd_data_blocks)/sb_format.s_block_size); i++) {
        if(i == 0) {
            free_blocks_list.push_back(bg_d_format.bgd_addr_first_free_block);
        } else {
            free_blocks_list.push_back(bg_d_format.bgd_addr_first_free_block + (i * 1024));
        }
    }

    // Escreve no Dispositivo (device), para criar uma
    // LISTA LIGADA DE BLOCOS LIVRES 
    uint32_t *x = free_blocks_list.data();

    for(int i = 0; i < free_blocks_list.size(); i++){
        std::fseek(device, free_blocks_list.at(i), SEEK_SET);
        std::fwrite(x+(i+1), sizeof(uint32_t), 1, device);
        if(i == (free_blocks_list.size() - 1)){
            std::fseek(device, free_blocks_list.at(i)+1020, SEEK_SET);
            std::fwrite(x+(i+1), sizeof(uint32_t), 1, device);
        }
    }
    uint32_t last_mask = 0xFFFFFFFF;    // Mascara para escrever no ultimo bloco livre 0xFFFF
    fseek(device, free_blocks_list.at(free_blocks_list.size() - 1), SEEK_SET);
    std::fwrite(&last_mask, sizeof(uint32_t), 1, device);

    // Cria as entradas primárias do ROOT "." e ".."
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

    // Atualiza a LISTA DE BLOCOS LIVRES, retirando o BLOCO alocado para o ROOT
    free_blocks_list.erase(free_blocks_list.begin());
    x = free_blocks_list.data();
    bg_d_format.bgd_addr_first_free_block = free_blocks_list[0];
    updateFreeBlockList(device, x);
}

void filesystem::mount(FILE* device){   // MONTAR DISPOSITIVO
    // Faz a leitura do Dispositivo, carregando a informações
    // necessárias para futuras operações.
    fread(sb, sizeof(superblock), 1, device);
    fread(bg_d, sizeof(blockgroup_descriptor), 1, device);
    
    uint32_t free_block;

    free_blocks_list.push_back(bg_d->bgd_addr_first_free_block);
    fseek(device, bg_d->bgd_addr_first_free_block, SEEK_SET);
    
    while(free_block != 0xFFFFFFFF) {
        fread(&free_block, sizeof(uint32_t), 1, device);
        if(free_block == 0x00000000 || free_block == 0xFFFFFFFF){
            free_block = 0xFFFFFFFF;
        }else{
            free_blocks_list.push_back(free_block);
            fseek(device, free_block, SEEK_SET);
        }
    }
}

void filesystem::listDirectory(FILE* device, uint32_t entry_inode, char* op){   // LISTAR DIRETÓRIOS/ARQUIVOS
    // Aponta na INODE TABLE onde está o respectivo inode e seus ponteiros.
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

void filesystem::makedir(FILE* device, std::string name, uint32_t entry_inode) {    // CRIAR DIRETÓRIOS
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
            if(entry.file_name[j] != 0 && (entry.file_type != 0xE5)) {
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
    /*  RESUMO DE PASSOS >>
        -------------------------------------------------------------------------------------------------------------------------------------------------------
        1 - Cria uma lista/pilha e coloca o Número do INODE do Diretório/Arquivo a ser Excluido
        2 - Procura na Inode Table a informação referente ao Tipo de Entrada do Inode existente na LISTA DE INODES
            2.1 - Se for Arquivo apenas continua para o proximo inode da LISTA
            2.2 - Senão se for Diretório, ir até os Ponteiros de Data Block, e ler as Entradas de Cada DATA BLOCK daquele Diretorio
        3 - Para cada entrada do diretório, colocar na pilha os inodes referentes a cada entrada;
        4 - Anda na lista, para o próximo Inode, repete o processo até que se chegue ao fim da lista, ou seja, nao tenha mais entradas para ler (DEEP SEARCH)
        5 - Nesse estado teremos uma pilha com todos os INODES, a partir disso começar a desempilhar
        6 - Para cada inode, deve-se:
            6.1 - Escrever 0xE5 nas entradas dos Data Blocks, para delimitar entrada/arquivo excluido;
            6.2 - Adicionar a Lista de Blocos Livres os Blocos Apontados pelo Inode;
            6.3 - Atualizar Inode_Bitmap com os inodes agora livres;
            6.4 - Zerar Inodes
        --------------------------------------------------------------------------------------------------------------------------------------------------------
    */

    uint8_t inode_type;
    uint32_t i_block_position[OWNFS_I_BLOCK_POINTERS];

    // 1
    std::vector<uint32_t> deep_search_vector;
    deep_search_vector.push_back(entry_inode);

    for (int i = 0; i < deep_search_vector.size(); i++){
        inode current_inode;
        uint32_t current_inode_number = deep_search_vector.at(i);

        // 2
        fseek(device, (this->bg_d->bgd_inode_table + (current_inode_number * 64)), SEEK_SET);
        fread(&current_inode, sizeof(inode), 1, device);
        
        // 2.1
        if(current_inode.i_type == 0x01){
            // É arquivo, portanto apenas continuar para excluir.
            continue;
        
        //2.2
        }else if(current_inode.i_type == 0x02){
            // 3 - Como é diretório deve-se salvar todas as suas entradas antes de ir para o próximo
            // 12 ponteiros direto para BLOCOS, portanto deve-se conferir todos
            for(int block_pointer = 0; block_pointer < 12; block_pointer++){
                // Se for "NULL" é porque nao esta sendo usado, então para
                if(current_inode.i_block[block_pointer] == 0x00000000){
                    break;
                }

                // Pular "." e ".."
                fseek(device, current_inode.i_block[block_pointer] + 64, SEEK_SET);
                
                // Ler entrada por entrada, jogando os INODES dela no VECTOR
                for(int index_entry = 64; index_entry < this->sb->s_block_size; index_entry+=32) {
                    dentry entry;
                    fread(&entry, sizeof(dentry), 1, device);

                    //Se inode = 0 é porque não possui entrada
                    if(entry.inode == 0x00000000 && entry.file_type == 0x00){
                        break;
                    }else{
                        deep_search_vector.push_back(entry.inode);
                    }
                }
            }  
        }else{
            std::cout << "ERROR MESSAGE (RMV FUNCTION :: LINE 323): CORRUPTED FILE " << std::endl;
            exit(-1);
        }
    }

    // 5 - VECTOR é a lista (deep_search) com todos os inodes que devem ser apagados
    for(int i = deep_search_vector.size(); i != 0; i--){
        inode deleted_inode;
        inode inode_parent;
        uint8_t filetype_mask = 0xE5;
        uint32_t deleted_inode_number = deep_search_vector.back();
        uint32_t inode_parent_number;
        
        if(deleted_inode_number == 0x00000000){
            break;
        }

        fseek(device, (this->bg_d->bgd_inode_table + (deleted_inode_number * 64)), SEEK_SET);
        fread(&deleted_inode, sizeof(inode), 1, device);


        // Se for arquivo, apenas apagar os inodes e liberar os Blocos, não precisa Escrever 0xE5
        // Se for diretorio, é preciso escrever 0xE5 nas entradas antes de apagar
        if(deleted_inode.i_type == 0x01){       //ARQUIVO
            
            writeFreeInodeBitmap(device, deep_search_vector.at(deep_search_vector.size() - 1));
            deep_search_vector.pop_back();

        }else if(deleted_inode.i_type == 0x02){ //DIRETORIO
            // 1 - Escrever E5 em todas as entradas do diretorio a ser excluido
            for(int current_pointer = 0; current_pointer < 12; current_pointer++){
                if(deleted_inode.i_block[current_pointer] == 0x00000000){
                    break;
                }

                for(int index_entry = 0; index_entry < this->sb->s_block_size; index_entry+=32) {
                    dentry entry;
                    
                    fseek(device, deleted_inode.i_block[current_pointer] + index_entry, SEEK_SET);
                    fread(&entry, sizeof(dentry), 1, device);

                    if(entry.inode == 0x00000000 && entry.file_type == 0x00){
                        break;
                    }else{
                        fseek(device, deleted_inode.i_block[current_pointer] + (index_entry + 7), SEEK_SET);
                        fwrite(&filetype_mask, sizeof(uint8_t), 1, device);
                    }
                }

                // ADICIONA NOVO BLOCO LIVRE A LISTA DE LIVRES
                free_blocks_list.push_back(deleted_inode.i_block[current_pointer]);
            }
            ///////////////////////////////////////////////////////////////////////
            
            // 2 - É preciso apagar as entradas dos diretorios que contem o diretorio apagado
            fseek(device, (deleted_inode.i_block[0] + 32), SEEK_SET);
            fread(&inode_parent_number, sizeof(uint32_t), 1,device);
            fseek(device, (this->bg_d->bgd_inode_table + (inode_parent_number * 64)), SEEK_SET);
            fread(&inode_parent, sizeof(inode), 1, device);

            for(int current_pointer = 0; current_pointer < 12; current_pointer++){
                
                if(inode_parent.i_block[current_pointer] == 0x00000000){
                    break;
                }

                for(int index_entry = 0; index_entry < this->sb->s_block_size; index_entry+=32) {
                    dentry entry;
                    fseek(device, inode_parent.i_block[current_pointer] + index_entry, SEEK_SET);
                    fread(&entry, sizeof(dentry), 1, device);
                    if(entry.inode == 0x00000000 && entry.file_type == 0x00){
                        break;
                    }else if(entry.inode == deleted_inode_number){
                        fseek(device, inode_parent.i_block[current_pointer] + (index_entry + 7), SEEK_SET);
                        fwrite(&filetype_mask, sizeof(uint8_t), 1, device);
                    }else{
                        continue;
                    }
                }
            }
            ///////////////////////////////////////////////////////////////////////////////////////
        }

        // Liberar os INODES
        writeFreeInodeBitmap(device, deep_search_vector.at(deep_search_vector.size() - 1));
        deep_search_vector.pop_back();
    }

    // Atualiza Blocos Livres
    uint32_t *x = free_blocks_list.data();
    updateFreeBlockList(device, x);
}

uint32_t filesystem::findDentryDir(FILE* device, std::string name, uint32_t inode) {    // Busca por diretórios e Retorna seu INODE
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

        if(!flag && (j == 24 || entry.file_name[j] == 0) && entry.file_type == 0x02) {
            return entry.inode;
        }
    }
    return -1;
}

uint32_t filesystem::findDentryFile(FILE* device, std::string name, uint32_t inode){    // Busca por arquivos e Retorna seu INODE
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

        if(!flag && (j == 24 || entry.file_name[j] == 0) && entry.file_type == 0x01) {
            return entry.inode;
        }
    }
    return -1;
}

uint32_t filesystem::seekFreeInode(FILE* device) {  // Busca por INODES LIVRES em INODE BITMAP
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

void filesystem::writeFreeInodeBitmap(FILE* device, uint32_t index_inode) {     // Escreve INODES LIVRES em INODE BITMAP
    uint32_t offset = index_inode / 8;
    uint32_t shift  = index_inode % 8;
    uint8_t  mask   = 0x80;
    uint8_t  entry  = 0x00;

    fseek(device, bg_d->bgd_inode_bitmap + offset, SEEK_SET);
    fread(&entry, sizeof(uint8_t), 1, device);
    entry ^= mask >> shift;
    fseek(device, bg_d->bgd_inode_bitmap + offset, SEEK_SET);
    fwrite(&entry, sizeof(uint8_t), 1, device);
}

void filesystem::writeInodeBitmap(FILE* device, uint32_t index_inode) {         // Escreve INODES OCUPADOS em INODE BITMAP
    uint32_t offset = index_inode / 8;
    uint32_t shift  = index_inode % 8;
    uint8_t  mask   = 0x80;
    uint8_t  entry  = 0x00;

    fseek(device, bg_d->bgd_inode_bitmap + offset, SEEK_SET);
    fread(&entry, sizeof(uint8_t), 1, device);
    entry |= mask >> shift;
    fseek(device, bg_d->bgd_inode_bitmap + offset, SEEK_SET);
    fwrite(&entry, sizeof(uint8_t), 1, device);
}

void filesystem::copy_file_HDtoFS(FILE* device, uint32_t dir_inode, std::string name_file, FILE* new_file){     // Copia do HD para o FS // NaF
    //FUNÇÀO SEM FUNCIONALIDADE
}

uint32_t filesystem::getBlockSize(){    // Retorna o tamanho do BLOCO == 1024
    return sb->s_block_size;
}

void filesystem::updateFreeBlockList(FILE* device, uint32_t *x){    // Atualiza a Lista de Blocos Livres
     
    uint32_t last_mask = 0xFFFFFFFF;

    // Altera o primeiro Bloco livre no Block Groups Descriptor
    std::fseek(device, sizeof(superblock) + 12, SEEK_SET );
    std::fwrite(x, sizeof(uint32_t), 1, device);

    //Reescreve na imagem a lista ligada de blocos livres
    for(int i = 0; i < free_blocks_list.size() - 1; i++){
        std::fseek(device, free_blocks_list.at(i), SEEK_SET);
        std::fwrite(x+(i+1), sizeof(uint32_t), 1, device);
    }

    std::fseek(device, free_blocks_list.at(free_blocks_list.size()-1), SEEK_SET);
    std::fwrite(&last_mask, sizeof(uint32_t), 1, device);
}

#endif // FILESYSTEM