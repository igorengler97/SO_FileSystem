#ifndef INODE_H
#define INODE_H

#define OWNFS_ROOT_INODE        0           // Inode do Diretório Raiz
#define OWNFS_TYPE_FILE         0x01
#define OWNFS_TYPE_DIR          0x02
#define OWNFS_TYPE_UNK          0xE5
#define OWNFS_I_BLOCK_POINTERS  14
#define OWNFS_I_UNUSED          3

#include <stdint.h>

// sizeof(inode) == 64 bytes
typedef struct inode {
    
    // Constructor
    inode();

    // Functions
    void writeFile(FILE *device);

    
    //  Variável de 16 bits usado para indicar o formato do arquivo.
    //
    //  -- formato do arquivo --
    //  OWNFS_TYPE_FILE    0x01  file
    //  OWNFS_TYPE_DIR     0x02  directory
    //  OWNFS_TYPE_UNK     0xE5  unknown
    uint8_t i_type;

    // Variável de 32 bits indicando o tamanho do arquivo em bytes
    uint32_t i_size;

    // Vetor de 14 Ponteiros para Blocos (de 32 bits) apontando para os 
    // blocos contendo dados.  As primeiras 12 entradas no vetor 
    // contém os números dos blocos de dados(ou seja, são entradas 
    // "diretas"). A 13ª entrada (i_block [12]) contém o número do 
    // bloco do primeiro bloco indireto, que contém ele próprio 
    // (tamanho do bloco / 4) o número de blocos dos blocos de dados.
    // A 14ª entrada (bloco i [13]) contém o número do primeiro bloco 
    // duplamente indireto.
    // 12 Diretos || 1 Indireto || 1 Duplamente Indireto
    uint32_t i_block[OWNFS_I_BLOCK_POINTERS];                   // Vetor de Índices

    // Não usado
    uint8_t i_unused[OWNFS_I_UNUSED];
    
} __attribute__((__packed__)) inode;

inode::inode(){
    this->i_type = OWNFS_TYPE_UNK;
    this->i_size = 0;
    std::fill_n(this->i_block, 14, NULL);
    std::fill_n(this->i_unused, 3, NULL);
}

void inode::writeFile(FILE* device){
    fwrite(this, sizeof(inode), 1, device);
}
#endif // INODE_H