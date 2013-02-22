#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define HR9_SIZE 	    620518328 // 134217728
#define READ_BLOCK_SIZE 500000000


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
	
    HR9 = (int *) malloc(HR9_SIZE * sizeof(int));
    ret = fread(HR9, sizeof(int), READ_BLOCK_SIZE, f);
    ret += fread(HR9 + READ_BLOCK_SIZE, sizeof(int), HR9_SIZE - READ_BLOCK_SIZE, f);
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