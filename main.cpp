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

    const char *name_device = "device1.own";

    int op, sectors;

    std::cout << "Operation: ";
    std::cin >> op;

    fs = new filesystem(sectors);

    switch(op) {
        case 1:
            device = fopen(name_device, "w+");
            if(device == NULL){
                std::cout << "ERROR: Not Open" << std::endl;
                return -1;
            }
            
            std::cout << "Number of Sectors: ";
            std::cin >> sectors;
            
            fs->format(device, sectors);
            fclose(device);
            break;
        
        default:
            std::cout << "Invalid Operation!" << std::endl;
    }

    std::cout << fs->getBlockSize() << std::endl;

    return 0;
}