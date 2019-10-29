#ifndef SUPERBLOCK_H
#define SUPERBLOCK_H

// Total 432 bits = 54 bytes
//
// Representação
// 2^6bits = 64 bytes (0 ~ 63)
//   SOBRA 32 BYTES
//
// As constantes a seguir são usadas em alguns dos campos de superbloco.
// Pesquise-os nos comentários para descobrir o que eles significam.

#define OWNFS_SUPER_MAGIC    0x0310
#define OWNFS_BLOCK_SIZE     1024
#define OWNFS_INODE_SIZE     64

#include <stdint.h>

typedef struct superblock {

    // Construtor
    superblock();
    superblock(int partition_size);

    // Funcoes
    void writeFile(FILE* file);

    // "Número mágico" identificando o sistema de arquivos como tipo EXT2.
    // esse valor é definido como EXT2_SUPER_MAGIC, que possui o valor 0xEF53
    uint16_t s_magic;

    // Número total de blocos, usados ​​e livres, no sistema.
    uint32_t s_blocks_count;

    // Número total de inodes, usados ​​e livres, no sistema.
    uint32_t s_inodes_count;

    // Número total de blocos livres.
    uint32_t s_free_blocks_count;

    // Número total de inodes livres.
    uint32_t s_free_inodes_count;

    // Localização do primeiro bloco de dados - ou seja, ID do bloco que
    // contém o superbloco.  É 0 para sistemas de arquivos com tamanho de 
    // bloco > 1 KB ou 1 para sistemas de arquivos com tamanho de bloco de
    // 1 KB.  Lembre-se de que o superbloco sempre inicia no 1024º byte do disco.
    uint32_t s_first_data_block;

    // log_2(blocksize/1024).  Portanto, o tamanho do bloco é calculado como:
    uint32_t s_block_size;

    // Tamanho de um inode. EXT2_INODE_SIZE (64 bytes).
    uint16_t s_inode_size;

    // Não utilizado
    uint8_t s_unused[4];

} __attribute__((__packed__)) superblock;

superblock::superblock(){

}

superblock::superblock(int partition_size) {
    
    this->s_magic = OWNFS_SUPER_MAGIC;
    this->s_blocks_count = ceil(partition_size / OWNFS_BLOCK_SIZE);
    this->s_inodes_count = ceil(partition_size / OWNFS_BLOCK_SIZE);
    this->s_free_blocks_count = s_blocks_count;
    this->s_free_inodes_count = s_inodes_count;
    this->s_first_data_block = 0;
    this->s_block_size = 0;    
    this->s_inode_size = OWNFS_INODE_SIZE;
}

void superblock::writeFile (FILE* file) {
    fwrite(this, sizeof(superblock), 1, file);
}

#endif // SUPERBLOCK_H