/*      
 *      Instruções
 *  COMPILAR TERMINAL: g++ -o main main.cpp
 *  EXECUTAR Linux   : ./main
 *  EXECUTAR Windows : main
 */

#include <iostream>
#include "filesystem_controller.h"

using namespace std;

int main() {
    filesystem fs;
    FILE* file;

    int op, sectors;

    cout << "Operation: ";
    cin >> op;

    switch(op) {
        case 1:
            cout << "Number of Sectors: ";
            cin >> sectors;
            fs.format(file, sectors);
            break;
        
        default:
            cout << "Invalid Operation!" << endl;
    }

    std::cout << fs.getBlockSize() << endl;

    return 0;
}