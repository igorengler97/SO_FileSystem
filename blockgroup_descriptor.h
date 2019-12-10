#ifndef BLOCKGROUP_DESCRIPTOR_TABLE_H
#define BLOCKGROUP_DESCRIPTOR_TABLE_H

#include <stdint.h>

//TAMANHO DO BLOCKGROUP_DESCRIPTOR == 16 bytes
typedef struct blockgroup_descriptor {
    
    // Constructor
    blockgroup_descriptor();
    blockgroup_descriptor(unsigned int superblock_size);

    // Functions
    void writeFile(FILE *device);

    // ID do bloco (de 32 bits) do primeiro bloco do "mapa de bits do
    // inode" para o grupo de blocos que este descritor representa.
    uint32_t bgd_inode_bitmap;          // Endereçamento Máximo: 8 * (blocksize * num_blocks)

    // ID do bloco (de 32 bits) do primeiro bloco da "tabela de inode" 
    // para o grupo de blocos que este descritor representa.
    uint32_t bgd_inode_table;

    // ID do bloco (de 32 bits) do primeiro bloco do "Bloco de Dados" 
    // para o grupo de blocos que este descritor representa.
    uint32_t bgd_data_blocks;

    // ID do bloco (de 32 bits) do primeiro bloco da "lista de 
    // blocos" para o grupo de blocos que este descritor representa.
    uint32_t bgd_addr_first_free_block;

} __attribute__((__packed__)) blockgroup_descriptor;

blockgroup_descriptor::blockgroup_descriptor() {

}

blockgroup_descriptor::blockgroup_descriptor(unsigned int superblock_size){
    this->bgd_inode_bitmap = superblock_size + sizeof(blockgroup_descriptor);
    this->bgd_inode_table = superblock_size + sizeof(blockgroup_descriptor) + (4*1024);
    this->bgd_data_blocks = superblock_size + sizeof(blockgroup_descriptor) + (4*1024) + (26215 * 64);
    this->bgd_addr_first_free_block = this->bgd_data_blocks;
}

void blockgroup_descriptor::writeFile (FILE* device) {
    fwrite(this, sizeof(blockgroup_descriptor), 1, device);
}

#endif // BLOCKGROUP_DESCRIPTOR_TABLE_H