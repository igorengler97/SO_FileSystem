#ifndef DIRECTORYENTRY_H
#define DIRECTORYENTRY_H

// Um diretório é um arquivo que contém uma lista de entradas. Cada entrada 
// contém um número de inode e um nome de arquivo. Quando um processo usa um
// nome de caminho, o código do kernel procura nos diretórios para encontrar 
// o número do inode correspondente. Após o nome ter sido convertido em um 
// número de inode, o inode é carregado na memória e usado por solicitações
// subsequentes.

// Constantes usadas em alguns dos campos de entrada do diretório.
#define NAME_LEN     24        //  Tamanho do nome em Bytes
#define UNKNOWN      0xE5      //  Tipo de Arquivo Desconhecido
#define FT_REG_FILE  0x01      //  Arquivo
#define FT_DIR       0x02      //  Diretório

#include <stdint.h>
#include <string>
#include <cstring>

// Essa estrutura representa uma entrada de diretório. Observe que o campo
// "nome" é na verdade comprimento variável no disco; para os fins desta
// estrutura, assumiremos o pior caso de NAME_LEN bytes.

// TAMANHO DE CADA DENTRY/DIRECTORY ENTRY == 32 bytes
typedef struct dentry {
    
    dentry();

    void setName(std::string file_name);
    
    // O número de inode de 32 bits do arquivo referido por esta entrada 
    // de diretório. Se o número do inode for zero, é o inode reservado para o root
    uint32_t inode;

    //Tamanho da dentry == 0x20 == 32, ocupa até 2 bytes
    uint16_t entry_len;

    // Variável não sinalizada de 8 bits, indicando quantos bytes de dados de 
    // caracteres estão no nome.
    uint8_t name_len;

    // Variável não sinalizada de 8 bits, indicando o tipo do arquivo. Pode ser
    // qualquer um dos seguintes:
    //
    // EXT2_FT_UNKNOWN   0xE5  Unknown File Type
    // EXT2_FT_REG_FILE  0x01  Regular File
    // EXT2_FT_DIR       0x02  Directory File
    uint8_t file_type;

    // O próprio nome do arquivo. Os nomes dos arquivos serão preenchidos
    // caracteres (ou seja, valor 0) para que sempre haja um múltiplo de 4
    // caracteres em uma entrada de diretório.  Mas apenas os primeiros
    // caracteres "name_len" fazem parte do nome do arquivo e você não pode
    // confiar na existência de um caractere NULL na entrada (por exemplo, 
    // se o nome do arquivo for realmente um múltiplo perfeito de 4).
    uint8_t file_name[NAME_LEN];

} __attribute__((__packed__)) dentry;

dentry::dentry() {
    this->inode = 0;
    this->entry_len = 0;
    this->name_len = 0;
    this->file_type = UNKNOWN;
    std::fill_n(this->file_name, NAME_LEN, NULL); 
}

void dentry::setName(std::string file_name){
    std::memcpy(this->file_name, file_name.data(), file_name.size() > (NAME_LEN-1) ? (NAME_LEN-1) : file_name.size());
}

#endif // DIRECTORYENTRY_H