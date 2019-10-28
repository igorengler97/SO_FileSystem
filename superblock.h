#ifndef SUPERBLOCK_H
#define SUPERBLOCK_H

// Total 416 bits = 52 bytes

// Representação
// 2^6bits = 64 bytes (0 ~ 63)
//   SOBRA 12 BYTES

// As constantes a seguir são usadas em alguns dos campos de superbloco.
// Pesquise-os nos comentários para descobrir o que eles significam.
#define EXT2_SUPER_MAGIC                 0xEF53
#define EXT2_GOOD_OLD_INODE_SIZE         128
#define EXT2_FEATURE_COMPAT_DIR_PREALLOC 0x0001

#include <stdint.h>

// Você deve encontrar que sizeof(struct superblock) == 592 bits == 128 bytes
typedef struct superblock {

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
    uint32_t s_log_block_size = 0;
    uint32_t blocksize = 1024 << s_log_block_size;

    // Número de blocos por grupo.  Combinado com s_first_data_block,
    // você pode usar isso para descobrir os limites do grupo de blocos.
    uint32_t s_blocks_per_group;

    // Número de inodes por group.  Também pode ser usado para determinar o 
    // tamanho do bitmap de inode de cada grupo de blocos.
    uint32_t s_inodes_per_group;

    // Última vez que o sistema de arquivos foi montado, em "horário UNIX" 
    // (# segundos desde a época).
    uint32_t s_mtime;

    // Última vez em que o sistema de arquivos foi gravado, em "horário UNIX"
    // (# segundos desde a época).
    uint32_t s_wtime;

    // "Número mágico" identificando o sistema de arquivos como tipo EXT2.
    // esse valor é definido como EXT2_SUPER_MAGIC, que possui o valor 0xEF53
    uint16_t s_magic;

    // Tamanho de um inode. Na revisão 0, isso é
    // EXT2_GOOD_OLD_INODE_SIZE (128 bytes).
    uint16_t s_inode_size;

    // Número do grupo de blocos que hospeda esse superbloco.  Pode ser usado
    // para reconstruir o sistema de arquivos a partir de qualquer backup de superbloco.
    uint16_t s_block_group_nr;

    // Variável de 128 bits usado como o ID de volume exclusivo. todo sistema de arquivos
    // no mundo deve receber um ID de volume exclusivo, se possível.
    uint8_t s_uuid[16];

    // Não utilizado
    uint8_t s_unused[66];

    // Construtor
    superblock(int sectors);

} __attribute__((__packed__)) superblock;

superblock::superblock(int sectors) {
    int tam_partition = sectors * 512;          // Em bytes

    s_first_data_block = 0;
    
    s_blocks_per_group = 8 * blocksize;
    s_inodes_per_group = 8 * blocksize;

    s_inodes_count = s_inodes_per_group * (tam_partition / ((blocksize * 8) * blocksize));
    s_free_inodes_count = s_inodes_count;

    s_blocks_count = tam_partiton / blocksize;
    s_free_blocks_count = s_blocks_count;

    s_blockgroup_count = tam_partition / ((blocksize * 8) * blocksize);
}

#endif // SUPERBLOCK_H