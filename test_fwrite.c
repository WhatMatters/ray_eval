#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define HR9_SIZE 	    620518328 // 134217728
#define READ_BLOCK_SIZE 2000000000 

int load_file(void* dest, size_t size, size_t nitems, FILE* stream)
{
    int ret = 0;
    size_t bytes_to_load = nitems * size;
    int block_size = 0;
    int offset = 0;
        
    while (bytes_to_load > 0)
    {
        block_size = bytes_to_load > READ_BLOCK_SIZE ? READ_BLOCK_SIZE : bytes_to_load;
        ret += fread(dest+offset, block_size, 1, stream);
        offset += block_size;
        bytes_to_load -= block_size;
    }
    
    return ret;
}


int main()
{
	setvbuf(stdout, NULL, _IONBF, 0);

	int *HR9 = (int *) malloc(HR9_SIZE * sizeof(int));
	memset((int *) HR9, 0, HR9_SIZE * sizeof(int));
    

	printf("generating...");
	int i, count1 = 0;
	for (i = 0; i < HR9_SIZE; i++)
		if (!(i % 12345))
			HR9[i] = i;
    
	printf(" -> writing...");
	FILE *f = fopen("test_fwrite.dat", "wb");
	fwrite(HR9, sizeof(int), HR9_SIZE, f);
	fclose(f);

	free(HR9);
	HR9 = 0;

	printf(" -> reading...");
    
	f = fopen("test_fwrite.dat", "rb");
    int ret = 0;
    int nret = 0;
    HR9 = (int *) malloc(HR9_SIZE * sizeof(int));
    ret = load_file(HR9, sizeof(int), HR9_SIZE, f);
	fclose(f);
    
    printf("\nret: %d\n", ret);

	int errors = 0;
	for (i = 0; i < HR9_SIZE; i++)
        if (!(i % 12345))
            if (HR9[i] != i)
                errors++;
        

    printf(" -> errors = %d -> %s\n", errors, errors > 0 ? "fail" : "success");

	free(HR9);
	HR9 = 0;

	return 0;
}