#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define HR9_SIZE 	620518328 // 134217728


int main()
{
	setvbuf(stdout, NULL, _IONBF, 0);

	int *HR9 = (int *) malloc(HR9_SIZE * sizeof(int));
	memset((int *) HR9, 0, HR9_SIZE * sizeof(int));

	printf("generating...");
	int i, count1 = 0;
	for (i = 0; i < HR9_SIZE; i++)
		if (!(i % 12345))
		{
			HR9[i] = i;
			count1++;
		}
	printf(" -> count1 = %d", count1);

	printf(" -> writing...");
	FILE *f = fopen("test_fwrite.dat", "wb");
	fwrite(HR9, sizeof(int), HR9_SIZE, f);
	fclose(f);

	free(HR9);
	HR9 = 0;

	printf(" -> reading...");
	HR9 = (int *) malloc(HR9_SIZE * sizeof(int));
	f = fopen("test_fwrite.dat", "rb");
	fread(HR9, sizeof(int), HR9_SIZE, f);
	fclose(f);

	int count2 = 0;
	for (i = 0; i < HR9_SIZE; i++)
		if (HR9[i])
			count2++;
	printf(" -> count2 = %d -> %s\n", count2, count2 == count1 ? "success" : "fail");

	free(HR9);
	HR9 = 0;

	return 0;
}