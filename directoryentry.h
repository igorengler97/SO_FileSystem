#ifndef DIRECTORYENTRY_H
#define DIRECTORYENTRY_H

// Constantes usadas em alguns dos campos de entrada do diretório.
#define EXT2_NAME_LEN     255
#define EXT2_FT_UNKNOWN   0  //  Tipo de Arquivo Desconhecido
#define EXT2_FT_REG_FILE  1  //  Arquivo
#define EXT2_FT_DIR       2  //  Diretório
#define EXT2_FT_FIFO      3  //  Arquivo Buffer (valor original 5)

// Essa estrutura representa uma entrada de diretório. Observe que o campo
// "nome" é na verdade comprimento variável no disco; para os fins desta
// estrutura, assumiremos o pior caso de EXT2_NAME_LEN bytes.

typedef struct dentry {
    // O número de inode de 32 bits do arquivo referido por esta entrada 
    // de diretório. Se o número do inode for zero, significa esta entrada de
    // diretório é inválida, e deve ser "ignorada". EXT2 usa uma entrada de 
    // diretório com um número de inode zero como terminador da lista ligada
    // de entradas de diretório em um arquivo de diretório.
    uint32_t inode;

    // O deslocamento não sinalizado de 16 bits da próxima entrada de diretório
    // no arquivo de diretório, relativo ao início desta entrada do diretório. 
    // Este campo terá um valor pelo menos igual ao tamanho do registro atual, 
    // mas talvez mais, se algum espaço no arquivo de diretório não for utilizado.
    uint16_t rec_len;

    // Variável não sinalizada de 8 bits, indicando quantos bytes de dados de 
    // caracteres estão no nome. Esse valor nunca deve ser maior que rec_len - 8.
    uint8_t name_len;

    // Variável não sinalizada de 8 bits, indicando o tipo do arquivo. Pode ser
    // qualquer um dos seguintes:
    //
    // EXT2_FT_UNKNOWN   0  Unknown File Type
    // EXT2_FT_REG_FILE  1  Regular File
    // EXT2_FT_DIR       2  Directory File
    // EXT2_FT_CHRDEV    3  Character Device
    // EXT2_FT_BLKDEV    4  Block Device
    // EXT2_FT_FIFO      5  Buffer File
    // EXT2_FT_SOCK      6  Socket File
    // EXT2_FT_SYMLINK   7  Symbolic Link
    uint8_t file_type;

    // O próprio nome do arquivo. Os nomes dos arquivos serão preenchidos
    // caracteres (ou seja, valor 0) para que sempre haja um múltiplo de 4
    // caracteres em uma entrada de diretório.  Mas apenas os primeiros
    // caracteres "name_len" fazem parte do nome do arquivo e você não pode
    // confiar na existência de um caractere NULL na entrada (por exemplo, 
    // se o nome do arquivo for realmente um múltiplo perfeito de 4).
    uint8_t file_name[EXT2_NAME_LEN];

} dentry;

#endif // DIRECTORYENTRY_H