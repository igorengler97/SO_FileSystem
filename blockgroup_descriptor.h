#ifndef BLOCKGROUP_DESCRIPTOR_TABLE_H
#define BLOCKGROUP_DESCRIPTOR_TABLE_H

#include <stdint.h>

// Uma block group descriptor table é um vetor de block group
// descriptors.
//
// Note que sizeof(blockgroup_descriptor) == 16 bytes

typedef struct blockgroup_descriptor {
    
    // ID do bloco (de 32 bits) do primeiro bloco da "lista de 
    // blocos" para o grupo de blocos que este descritor representa.
    uint32_t bgd_block_list;

    // ID do bloco (de 32 bits) do primeiro bloco do "mapa de bits do
    // inode" para o grupo de blocos que este descritor representa.
    uint32_t bgd_inode_bitmap;

    // ID do bloco (de 32 bits) do primeiro bloco da "tabela de inode" 
    // para o grupo de blocos que este descritor representa.
    uint32_t bgd_inode_table;

    // ID do bloco (de 32 bits) do primeiro bloco do "Bloco de Dados" 
    // para o grupo de blocos que este descritor representa.
    uint32_t bgd_data_blocks;

} __attribute__((__packed__)) blockgroup_descriptor;

#endif // BLOCKGROUP_DESCRIPTOR_TABLE_H