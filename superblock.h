#ifndef SUPERBLOCK_H
#define SUPERBLOCK_H

#define OWNFS_SUPER_MAGIC    0x0310
#define OWNFS_BLOCK_SIZE     1024
#define OWNFS_INODE_SIZE     64

#include <stdint.h>

// Sizeof(superblock) == 32 bytes
typedef struct superblock {

    // Construtor
    superblock();
    superblock(int partition_size);

    // Funcoes
    void writeFile(FILE* device);
    void printSuperblock();


    // "Número mágico" identificando o sistema de arquivos como tipo EXT2.
    // esse valor é definido como OWNFS_SUPER_MAGIC, que possui o valor 0x0310
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

    // Tamanho de um inode. OWN_INODE_SIZE (64 bytes).
    uint16_t s_inode_size;

    // Não utilizado
    uint8_t s_unused[4];

} __attribute__((__packed__)) superblock;

superblock::superblock(){
    this->s_magic = OWNFS_SUPER_MAGIC;
    this->s_blocks_count = 0;
    this->s_inodes_count = 0;
    this->s_free_blocks_count = s_blocks_count;
    this->s_free_inodes_count = s_inodes_count;
    this->s_first_data_block = 0;
    this->s_block_size = OWNFS_BLOCK_SIZE;    
    this->s_inode_size = OWNFS_INODE_SIZE;
    std::fill_n(this->s_unused, 4, NULL);
}

superblock::superblock(int partition_size) {
    
    this->s_magic = OWNFS_SUPER_MAGIC;
    this->s_blocks_count = ceil(partition_size / OWNFS_BLOCK_SIZE);
    this->s_inodes_count = ceil(partition_size / OWNFS_BLOCK_SIZE);
    this->s_free_blocks_count = s_blocks_count;
    this->s_free_inodes_count = s_inodes_count;
    this->s_first_data_block = 0;
    this->s_block_size = OWNFS_BLOCK_SIZE;    
    this->s_inode_size = OWNFS_INODE_SIZE;
    std::fill_n(this->s_unused, 4, NULL);
}

void superblock::writeFile (FILE* device) {
    fwrite(this, sizeof(superblock), 1, device);
}

void superblock::printSuperblock (){
    std::cout << std::hex << this->s_magic << std::dec << std::endl;
    std::cout << this->s_blocks_count << std::endl;
    std::cout << this->s_inodes_count << std::endl;
    std::cout << this->s_free_blocks_count << std::endl;
    std::cout << this->s_free_inodes_count << std::endl;
    std::cout << this->s_first_data_block << std::endl;
    std::cout << this->s_block_size << std::endl;
    std::cout << this->s_inode_size << std::endl;
}

#endif // SUPERBLOCK_H