#include <iostream>
#include "filesystem_controller.h"

using namespace std;

int main() {
    filesystem fs;

    std::cout << fs.getBlocksize() << endl;

    return 0;
}