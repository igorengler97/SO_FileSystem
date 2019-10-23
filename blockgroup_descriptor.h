#ifndef BLOCKGROUP_DESCRIPTOR_TABLE_H
#define BLOCKGROUP_DESCRIPTOR_TABLE_H

#include <stdint.h>

// Uma block group descriptor table é um vetor de block group
// descriptors.
//
// Note que sizeof(blockgroup_descriptor) == 14 bytes + struct block.

typedef struct blockgroup_descriptor {
    
    // ID do bloco (de 32 bits) do primeiro bloco da "lista de 
    //blocos" para o grupo de blocos que este descritor representa.
    // uint32_t bg_block_bitmap;
    typedef struct block {
            
        // informação se o bloco está em uso ou  (COLOCAR DENTRO DO BLOCO???)
        bool isUsed;

        struct block *next;
    } block;

    // ID do bloco (de 32 bits) do primeiro bloco do "mapa de bits do
    // inode" para o grupo de blocos que este descritor representa.
    uint32_t bg_inode_bitmap;

    // ID do bloco (de 32 bits) do primeiro bloco da "tabela de inode" 
    // para o grupo de blocos que este descritor representa.
    uint32_t bg_inode_table;

    // Variável de 16 bits, indicando o número total de blocos livres 
    // no grupo de blocos que este descritor representa.
    uint16_t bg_free_blocks_count;

    // Variável de 16 bits indicando o número total de inodes livres 
    // no grupo de blocos que este descritor representa.
    uint16_t bg_free_inodes_count;

    // Variável de 16 bits indicando o número de inodes que foram alocados 
    // para diretórios no grupo de blocos que este descritor representa.
    uint16_t bg_used_dirs_count;

} blockgroup_descriptor;

#endif // BLOCKGROUP_DESCRIPTOR_TABLE_H