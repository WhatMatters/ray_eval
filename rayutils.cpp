
#include <stdio.h>

#include <iostream>
#include <fstream>

#include "rayutils.h"
#include "arrays.h"

// this function is required to read binary files larger than 2GB
int load_file(char* dest, size_t size, size_t nitems, FILE* stream)
{
    int ret = 0;
    size_t bytes_to_load = nitems * size;
    size_t block_size = 0;
    int offset = 0;
    
    while (bytes_to_load > 0)
    {
        block_size = bytes_to_load > READ_BLOCK_SIZE ? READ_BLOCK_SIZE : bytes_to_load;
        ret += fread(dest + offset, block_size, 1, stream);
        offset += block_size;
        bytes_to_load -= block_size;
    }
    
    return ret;
}

// binary search in the cactus array
int cactus_findit(int key)
{
    int low = 0, high = 4887, mid;
    while (low <= high)
    {
        mid = (high + low) >> 1;
        if (key < products[mid])
            high = mid - 1;
        else if (key > products[mid])
            low = mid + 1;
        else
            return mid;
    }
    std::cout << "\ncactus_findit(): no match found; key = " << key << "\n";
    return -1;
}

// convert cactus handrank to the new format
int cactus_to_ray(int holdrank)
{
    // hhhhrrrrrrrrrrrr   hhhh = 1 high card -> 9 straight flush
    //                    r..r = rank within the above  1 to max of 2861

	int handrank = 7463 - holdrank;  // now the worst hand = 1
		
	if      (handrank < 1278) handrank = handrank -    0 + 4096 * 1;  // 1277 high card
	else if (handrank < 4138) handrank = handrank - 1277 + 4096 * 2;  // 2860 one pair
	else if (handrank < 4996) handrank = handrank - 4137 + 4096 * 3;  //  858 two pair
	else if (handrank < 5854) handrank = handrank - 4995 + 4096 * 4;  //  858 three-kind
	else if (handrank < 5864) handrank = handrank - 5853 + 4096 * 5;  //   10 straights
	else if (handrank < 7141) handrank = handrank - 5863 + 4096 * 6;  // 1277 flushes
	else if (handrank < 7297) handrank = handrank - 7140 + 4096 * 7;  //  156 full house
	else if (handrank < 7453) handrank = handrank - 7296 + 4096 * 8;  //  156 four-kind
	else                      handrank = handrank - 7452 + 4096 * 9;  //   10 straight-flushes

	return handrank;	
}

const char *hand_rank_str(int handrank_num)
{
    const char *_hand_rank_str[] = 
    {
        "n/a",
        "straight flush",
        "four of a kind",
        "full house",
        "flush",
        "straight",
        "three of a kind",
        "two pairs",
        "one pair",
        "high card"
    };    
    return _hand_rank_str[handrank_num];
}

const char *get_hand_rank(int handrank)
{
    return hand_rank_str(handrank >> 12);
}


unsigned long mix(unsigned long a, unsigned long b, unsigned long c)
{
    a = a - b;  a = a - c;  a = a^(c >> 13);
    b = b - c;  b = b - a;  b = b^(a << 8);
    c = c - a;  c = c - b;  c = c^(b >> 13);
    a = a - b;  a = a - c;  a = a^(c >> 12);
    b = b - c;  b = b - a;  b = b^(a << 16);
    c = c - a;  c = c - b;  c = c^(b >> 5);
    a = a - b;  a = a - c;  a = a^(c >> 3);
    b = b - c;  b = b - a;  b = b^(a << 10);
    c = c - a;  c = c - b;  c = c^(b >> 15);
    return c;
}

static int RAND_MAX_DIV_52[53];

void init_random_int_52()
{
    int i;
    for (i = 1; i <= 52; i++)
        RAND_MAX_DIV_52[i] = RAND_MAX / (i + 1);
    srand((unsigned int) mix(clock(), time(NULL), getpid()));
}

int random_int_52(int k) 
{
    int r;
    if (k == 0)
        return 0; // prevent potential segfault
    do { r = rand() / RAND_MAX_DIV_52[k]; } while (r > k);
    return r; // 0 to k
}

inline void swap(int *x, int *y)
{
    int z = *x;
    *x = *y;
    *y = z;
}

void random_sample_52_ross(int n, int k, int *out)
{
    // Ross algorithm modified to work in-place (C) Aldanor
    const int DECK_52[52] = {
         0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12,
        13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,
        26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38,
        39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51};
    memcpy(out, DECK_52, sizeof(DECK_52));
    int i;
    for (i = 0; i < k; i++)
        swap(out + i, out + i + random_int_52(n - 1 - i));
}

int *smart_load(const char *filename)
{
    // the size of the file is contained in the header
    FILE *f = fopen(filename, "rb");
    if (f)
    {
        int size = 0;
        fread(&size, sizeof(int), 1, f);
        int *data = (int *) malloc(size * sizeof(int));
        load_file((char *)data, sizeof(int), size, f);
        fclose(f);
        return data;
    }
    else
        return NULL;
}

int smart_save(int *x, int size, const char *filename)
{
    std::cout << "\nSaving the data to \"" << filename << "\"...";
    std::ofstream out(filename, std::ios::out | std::ios::binary);
    if (!out)
    {
        std::cout << "\tError opening file.\n";
        return 1;
    }
    int header[1];
    header[0] = size; // store the size in the first 4 bytes
    out.write(reinterpret_cast<const char *>(header), sizeof(int));
    out.write(reinterpret_cast<const char *>((int *) x), 
        sizeof(int) * header[0]);
    return 0;    
}

