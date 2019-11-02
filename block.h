#ifndef BLOCK_H
#define BLOCK_H

#include <stdint.h>

typedef struct block {
    uint32_t data;
    struct block *next;
} __attribute__((__packed__)) block;

#endif // BLOCK_H