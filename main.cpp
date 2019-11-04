/*      
 *      Instruções
 *  COMPILAR TERMINAL: g++ -o main main.cpp
 *  EXECUTAR Linux   : ./main
 *  EXECUTAR Windows : main
 */

#include <iostream>
#include "filesystem_controller.h"

int main() {
    filesystem *fs;
    FILE* device;

    const char *name_device = "device256MiB.own";
    int op;
    
    std::cout << "Operation: ";
    std::cin >> op;

    fs = new filesystem();

    if(op == 1) {           // FORMAT
        int sectors;

        device = fopen(name_device, "w+");
        if(device == NULL){
            std::cout << "ERROR: Not Open (FORMAT)." << std::endl;
            return -1;
        }
        
        std::cout << "Number of Sectors: ";
        std::cin >> sectors;
        
        fs->format(device, sectors);

        fclose(device);

    } else if(op == 2) {    // MAKE DIR
        uint32_t inode = -1;
        char *token, *str, *tofree;

        device = fopen(name_device, "r+");
        if(device == NULL){
            std::cout << "ERROR: Not Open (MAKE DIR)." << std::endl;
            return -1;
        }

        fs->mount(device);
        tofree = str = strdup("/folder1/folder2");
        
        while((token = strsep(&str, "/"))) {
            if(!strcmp(token, "")) {                    // Se a entrada for somente "/", ou seja, o ROOT
                inode = 0;

            } else if (strstr(token, ".") != NULL) {
                std::cout << "ERROR: Already exists (MAKE DIR)" << std::endl;
                exit(-1);   // Sai do WHILE
            
            } else {
                uint32_t new_inode = fs->findDentryDir(device, std::string(token), inode);
                if(new_inode == -1) {
                    fs->makedir(device, std::string(token), inode);
                    inode = fs->findDentryDir(device, std::string(token), inode);
                } else {
                    inode = new_inode;
                }
            }
        }

        free(tofree);

        fclose(device);
    } else if(op == 3) {    // Copy File HD to FS
        uint32_t inode = -1;
        char *token, *str, *tofree;

        uint32_t inode_copy = -1;
        uint32_t inode_copy_ant = -1;
        char *token_copy, *str_copy, *tofree_copy;
        char *name_copy = NULL, *extension_copy = NULL;
		char *local_token_copy, *local_str_copy, *local_tofree_copy;

        std::pair<uint32_t, uint32_t> inode_entry_info;

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
                    inode = new_inode;
                }
            }
        }

        tofree_copy = str_copy = strdup("/teste.txt");
        while((token_copy = strsep(&str_copy, "/"))){
            if(!strcmp(token_copy, "")){
                inode_copy = 0;
            }else if (strstr(token_copy, ".") != NULL){
                local_tofree_copy = local_str_copy = strdup(token_copy);
                while((local_token_copy = strsep(&local_str_copy, "."))){
                    if (name_copy == NULL){
                        name_copy = local_token_copy;
                    }else{
                        extension_copy = local_token_copy;
                        break;
                    }
                }

                inode_entry_info = fs->findDentryFile(device, std::string(name_copy), std::string(extension_copy), inode_copy);
                free(local_tofree_copy);
                break;
            }else{
                uint32_t new_inode_copy = fs->findDentryDir(device, std::string(token_copy), inode_copy);
                if(new_inode_copy == -1) {
                    fs->makedir(device, std::string(token_copy), inode_copy);
                    inode_copy = fs->findDentryDir(device, std::string(token_copy), inode_copy);
                } else {
                    inode_copy = new_inode_copy;
                }
            }
        }

        fs->copy_file_HDtoFS(device, inode, inode_entry_info.first, inode_entry_info.second);
        std::cout << "File copied succesfully!" << std::endl;
        
        free(tofree_copy);

        fclose(device);
    }else{
        std::cout << "Invalid Operation!" << std::endl;
    }

    //std::cout << fs->getBlockSize() << std::endl;

    return 0;
}