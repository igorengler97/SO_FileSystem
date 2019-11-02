#ifndef INODE_BITMAP_H
#define INODE_BITMAP_H

#include <stdint.h>

typedef struct inode_bitmap {
    
    // Constructor
    inode_bitmap();

    // Functions
    void writeFile(FILE *device);

    uint8_t ib_bitmap[4*1024];

} __attribute__((__packed__)) inode_bitmap;

inode_bitmap::inode_bitmap(){
    std::fill_n(this->ib_bitmap, (4*1024), NULL);
}

void inode_bitmap::writeFile (FILE* device) {
    fwrite(this, sizeof(inode_bitmap), 1, device);
}

#endif // INODE_BITMAP_H