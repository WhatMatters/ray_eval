#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mach/mach.h>
#include <mach/mach_time.h>

// recommended compile flags: -O3 -msse4 -fPIC -m64

double get_time(uint64_t end, uint64_t start)
{
	uint64_t difference = end - start;
	static double conversion = 0.0;
	if (conversion == 0.0)
	{
		mach_timebase_info_data_t info;
		kern_return_t err = mach_timebase_info( &info );
		if (err == 0)
			conversion = 1e-9 * (double)info.numer / (double)info.denom;
	}
	return conversion * (double)difference;
}

int RAND_MAX_DIV_52[53];

void init_random_int_52(void)
{
	int i;
	for (i = 1; i <= 52; i++)
		RAND_MAX_DIV_52[i] = RAND_MAX / (i + 1);
	srand(time(NULL));
}

int random_int_52(int k) 
{
    int r;
	do 
	{
		r = rand() / RAND_MAX_DIV_52[k];
    } while (r > k);
    return r;
}

// Reservoir sampling
// http://en.wikipedia.org/wiki/Reservoir_sampling
// Can we do any faster?
// Sampling via weighted tree?
// http://www.stdlib.net/~colmmacc/2011/05/15/weighty-matters/
void random_sample_52(int n, int k, int *out)
{
	int i, r;
	out[0] = 0;
	for (i = 1; i < k; i++)
	{
		r = random_int_52(i);
		out[i] = out[r];
		out[r] = i;
	}
	for (i = k; i < n; i++)
	{
		r = random_int_52(i);
		if (r < k)
			out[r] = i;
	}
}

int main(void)
{
	const int N = 1e7, N2 = 1e6;
	const int n = 52;
	const int k = 15;
	int i, j, out[n];
	int counts[n];

	init_random_int_52();
	printf("Generating %d random samples (%d out of %d)...\n", N, k, n);
	uint64_t start = mach_absolute_time();
	for (i = 0; i < N; i++)
		random_sample_52(n, k, out);
	double elapsed = get_time(mach_absolute_time(), start);

	printf("Verifying counts...\n");
	for (i = 0; i < N2; i++)
	{
		random_sample_52(n, k, out);
		for (j = 0; j < k; j++)
			counts[out[j]]++;
	}

	for (i = 0; i < n; i++)
		printf("%6d: %.6f%%\n", i, 100.0 * ((double)counts[i] / k) / (double)N2);

	printf("\nRandom sampling took %.4f seconds (%.0f samples / sec).\n", elapsed, (N / elapsed));

	return 0; 
}