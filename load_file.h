#define READ_BLOCK_SIZE 2000000000

int load_file(char* dest, size_t size, size_t nitems, FILE* stream)
{
    int ret = 0;
    size_t bytes_to_load = nitems * size;
    int block_size = 0;
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