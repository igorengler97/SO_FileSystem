/*      
 *      Instruções
 *  COMPILAR TERMINAL: g++ -o main main.cpp
 *  EXECUTAR Linux   : ./main
 *  EXECUTAR Windows : main
 */

#include <iostream>
#include "filesystem_controller.h"


int main(int argc, char *argv[]) {
    
    filesystem *fs = new filesystem();
    FILE* device;

    char* name_device;
    char* op;
    
    if (argc > 1){
        name_device = argv[1];
    }else{
        std::cout << "ERROR MESSAGE: Invalid Arguments." << std::endl;
        exit(-1);
    }
    
    if(argc > 2) {           
        op = argv[2];

        if(!strcmp(op, "format")){                  //FORMAT - format
            strcat(name_device, ".own");
            device = fopen(name_device, "w+");
            if(device == NULL){
                std::cout << "ERROR: Not Open (FORMAT)." << std::endl;
                return -1;
            }
            
            int sectors;
            std::cout << "Number of Sectors: ";
            std::cin >> sectors;
            fs->format(device, sectors);
            fclose(device);
            std::cout << "Successfully Formatted Device." << std::endl;
            std::cout << "Disk Size: " << ((sectors * 512)/1024)/1024 << "MiB" << std::endl;

        }else{
            name_device = argv[1];
			device = fopen( name_device, "r+" );
			if(device == NULL){
                std::cout << "ERROR: Not Open (FOPEN R+)." << std::endl;
                return -1;
            }
            
            
            if(!strcmp(op, "ls")){                  //List Directory - ls
                if(argc > 3){
                    uint32_t inode = -1;
                    char *token, *str, *tofree;

                    fs->mount(device);

                    tofree = str = strdup(argv[3]);

                    while((token = strsep(&str, "/"))) {
                        if(!strcmp(token, "")) {                    // Se a entrada for somente "/", ou seja, o ROOT
                            inode = 0;

                        } else if (strstr(token, ".") != NULL) {
                            std::cout << "ERROR: Invalid Argument (LS)" << std::endl;
                            exit(-1);   // Sai do WHILE
                    
                        } else {
                            uint32_t new_inode = fs->findDentryDir(device, std::string(token), inode);
                            if(new_inode == -1) {
                                std::cout << "Directory not found!" << std::endl;
                                exit(-1);
                            }else {
                                inode = new_inode;
                            }
                        }
                    }
                    fs->listDirectory(device, inode, op);
                    free(tofree);
                    fclose(device);
                }
            }else if(!strcmp(op, "makedir")){       // Create Directory - makedir
                if(argc > 3){
                    uint32_t inode = -1;
                    char *token, *str, *tofree;

                    fs->mount(device);

                    tofree = str = strdup(argv[3]);
                
                    while((token = strsep(&str, "/"))) {
                        if(!strcmp(token, "")) {
                            inode = 0;

                        } else if (strstr(token, ".") != NULL) { 
                            std::cout << "ERROR: Invalid Argument (MAKE DIR)" << std::endl;
                            exit(-1);   // Sai do WHILE
                    
                        } else {
                            uint32_t new_inode = fs->findDentryDir(device, std::string(token), inode);
                            if(new_inode == -1) {
                                fs->makedir(device, std::string(token), inode);
                                inode = fs->findDentryDir(device, std::string(token), inode);
                            }else {
                                inode = new_inode;
                            }
                        }
                    }

                    free(tofree);

                    fclose(device);
                }
            }else if(!strcmp(op, "rmv")){            // Remove Directory, subdirectories and files - rmv
                /* 01 - Ver todas as entradas que o Diretório a ser excluido possui
                        salvar os inodes referentes a todas as entradas, fazer o mesmo
                        recursivamente para todos os subdiretorios no diretorio alvo
                        ao final teremos uma lista de todos os inodes, de todos os
                        diretorios e subdiretorios do diretorio alvo, dessa forma 
                        podemos preencher todos os blocos referenciados com 0, é 
                        necessaŕio tamber criar uma lista com a posicao de todos os blocos
                        que serão liberados, para poder adiciona-los a lista de blocos livres
                        apos isso e necessario apagar as informacoes nos blocos, nos inodes,
                        e entao adicionar os novos blocos livres a lista e liberar os inode bitmaps
                */
                
                if(argc > 3){
                    uint32_t inode = -1;
                    uint32_t parent_inode = -1;
                    char *token, *str, *tofree;

                    fs->mount(device);

                    tofree = str = strdup(argv[3]);

                    while((token = strsep(&str, "/")))  {
                        
                        if(!strcmp(token, "")) {
                            inode = 0;
                        } else if (strstr(token, ".") != NULL) { //Se verdadeiro quer dizer que é arquivo
                            //uint32_t new_inode = fs->findDentryDir(device, std::string(token), inode);
                            // Acima: Procura o inode referente a entrada de arquivo 

                            // FAZER UMA VARIAVEL ANT QUE RECEBE A PASTA ANTERIOR AO ARQUIVO, E PASSA O INODE DELA PARA EXCLUIR A ENTRADA DO ARQUIVO

                            std::cout << "MESSAGE (RMV): FILE!" << std::endl;
                            parent_inode = inode;
                            uint32_t new_inode = fs->findDentryFile(device, std::string(token), inode);
                            inode = new_inode;
                            break;
                        } else {
                            parent_inode = inode;
                            uint32_t new_inode = fs->findDentryDir(device, std::string(token), inode);
                            if(new_inode == -1) {
                                std::cout << "Directory not found!" << std::endl;
                                inode = -1;
                                exit(-1);
                            }else {
                                parent_inode = inode;
                                inode = new_inode;
                            }
                        }
                    }
                    
                    if(inode != -1){
                        fs->rmv(device, inode, parent_inode);
                    }

                    free(tofree);
                    fclose(device);
                }

            }else if(!strcmp(op, "cpy_hdtofs")) {   // Copy File HD to FS
                if(argc > 4){
                    exit(-1);
                }

            }else{
                std::cout << "Invalid Operation!" << std::endl;
            }
    
        }
    }
}