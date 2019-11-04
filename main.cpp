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

        if(!strcmp(op, "format")){              //FORMAT - format
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

        }else{
            name_device = argv[1];
			device = fopen( name_device, "r+" );
			if(device == NULL){
                std::cout << "ERROR: Not Open (FOPEN R+)." << std::endl;
                return -1;
            }
            
            
            if(!strcmp(op, "ls")){              //List Directory - ls
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
                            }else {
                                inode = new_inode;
                            }
                        }
                    }
                    fs->listDirectory(device, inode, op);
                    free(tofree);
                    fclose(device);
                }
            }else if(!strcmp(op, "ls-all")){    // List ALL - ls-all
                if(argc > 3){
                    uint32_t inode = -1;
                    char *token, *str, *tofree;

                    fs->mount(device);

                    tofree = str = strdup(argv[3]);

                    while((token = strsep(&str, "/"))) {
                        if(!strcmp(token, "")) {                    // Se a entrada for somente "/", ou seja, o ROOT
                            inode = 0;

                        } else if (strstr(token, ".") != NULL) {
                            std::cout << "ERROR: Invalid Argument (LS-ALL)" << std::endl;
                            exit(-1);   // Sai do WHILE
                    
                        } else {
                            uint32_t new_inode = fs->findDentryDir(device, std::string(token), inode);
                            if(new_inode == -1) {
                                std::cout << "Directory not found!" << std::endl;
                            }else {
                                inode = new_inode;
                            }
                        }
                    }
                    fs->listDirectory(device, inode, op);
                    free(tofree);
                    fclose(device);
                }
            }else if(!strcmp(op, "makedir")){   // Create Directory - makedir
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
            }else if(!strcmp(op, "cpy_hdtofs")) {    // Copy File HD to FS
                if(argc > 4){
                    uint32_t inode = -1;
                    char *token, *str, *tofree;

                    fs->mount(device);

                    //- Encontra o inode do diretorio de Entrada do Arquivo
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
                    //-
                    
                    FILE* new_file = fopen(argv[4],"r");
                    //precisar dar um jeito de pegar o nome do arquivo pelo path/arg

                    char *namef;
                    namef = strdup("teste.txt");

                }
            /*
            uint32_t inode = -1;
            char *token, *str, *tofree;

            device = fopen(name_device, "r+");
            if(device == NULL){
                std::cout << "ERROR: Not Open (Copy HD->FS)." << std::endl;
                return -1;
            }

            fs->mount(device);
            
            tofree = str = strdup("/folder1");
            
            while((token = strsep(&str, "/"))) {
                if(!strcmp(token, "")) {                    // Se a entrada for somente "/", ou seja, o ROOT
                    inode = 0;

                } else if (strstr(token, ".") != NULL) {
                    std::cout << "ERROR: Already exists (Copy HD->FS)" << std::endl;
                    exit(-1);   // Sai do WHILE
                
                } else {
                    uint32_t new_inode = fs->findDentryDir(device, std::string(token), inode);
                    if(new_inode == -1) {
                        std::cout << "ERROR: Directory not found! (Copy HD->FS)" << std::endl;
                        return -1;
                    } else {
                        inode = new_inode; //Inode referente a pasta onde sera salvo o arquivo
                    }
                }
            }
        
            FILE* cpy_file = fopen("teste.txt", "r");
        

            fclose(device);*/
        }else{
            std::cout << "Invalid Operation!" << std::endl;
        }

        //std::cout << fs->getBlockSize() << std::endl;

        return 0;
    
        }
    }
}   