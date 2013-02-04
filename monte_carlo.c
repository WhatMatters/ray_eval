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

// Generate a random integer from [0, k],
// i.e. k + 1 distinct values
int random_int_52(int k) 
{
    int r;
	do 
	{
		r = rand() / RAND_MAX_DIV_52[k];
    } while (r > k);
    return r;
}

// Reservoir sampling : http://en.wikipedia.org/wiki/Reservoir_sampling
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

const int DECK_52[52] = {
	 0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12,
	13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,
	26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38,
	39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51};

static void swap(int *x, int *y)
{
	int z = *x;
	*x = *y;
	*y = z;
}

static int max(int x, int y)
{
	return x > y ? x : y;
}

// Ross algorithm modified to work in-place (C) Aldanor
// This is 4.5x faster than partial Fisher-Yates / reservoir
void random_sample_52_ross(int n, int k, int *out)
{
	memcpy(out, DECK_52, sizeof(DECK_52));
	int i;
	for (i = 0; i < k; i++)
		swap(out + i, out + i + random_int_52(n - 1 - i));
}

const uint64_t ONE_64 = 1LLU;

static void extract_cards(uint64_t *deck, int card)
{
	*deck ^= (ONE_64 << card);
}

static void get_cards(uint64_t deck, int *cards, int shift)
{
	int i, pos = 0;

	for (i = 0; i < 52; i++)
		if (deck & (ONE_64 << i))
			cards[pos++] = i + shift;
}

uint64_t new_deck()
{
	uint64_t deck = 0;
	int i;

	for (i = 0; i < 52; i++)
		deck |= (ONE_64 << i);
	return deck;
}

#define MAX_PLAYERS 10

int eval_monte_carlo_holdem(const int *HR, int N, int *board, int n_board, 
	int *pocket, int n_players, double *ev)
{
	int mask[52], cards[52], n_cards = 0, n_mask = 0, n_available, i, j, k;
	memset(mask, 0, 52 * sizeof(int));
	memset(cards, 0, 52 * sizeof(int));
	memset(ev, 0, n_players * sizeof(double));
	uint64_t deck = new_deck();
	int available_cards[52];
	if (n_board != 3 && n_board != 4 && n_board != 5)
		return 1;
	for (i = 0; i < n_board; i++)
	{
		if (board[i] == 0)
			mask[n_mask++] = n_cards;
		else
			extract_cards(&deck, board[i]);
		cards[n_cards++] = board[i] + 1; // convert 0-51 to 1-52
	}
	for (i = 0; i < 2 * n_players; i++)
	{
		if (pocket[i] == 0)
			mask[n_mask++] = n_cards;
		else
			extract_cards(&deck, pocket[i]);
		cards[n_cards++] = pocket[i] + 1; // convert 0-51 to 1-52
	}
	n_available = 52 - n_board - 2 * n_players + n_mask;
	get_cards(deck, available_cards, 1); // convert 0-51 to 1-52
	init_random_int_52();
	for (i = 0; i < N; i++)
	{
		int sample[52], scores[MAX_PLAYERS], best_score = -1, tied = 0;
		random_sample_52_ross(n_available, n_mask, sample);
		for (j = 0; j < n_mask; j++)
			cards[mask[j]] = available_cards[sample[j]];
		int path = 53;
		for (j = 0; j < n_board; j++)
			path = HR[path + cards[j]];
		int *player_cards = cards + n_board;
		for (k = 0; k < n_players; k++)
		{
			int score = HR[HR[path + player_cards[0]] + player_cards[1]];
			scores[k] = score;
			if (score > best_score)
			{
				best_score = score;
				tied = 1;
			}
			else if (score == best_score)
				tied++;
			player_cards += 2;
		}
		double delta_ev = 1.0 / tied;
		for (k = 0; k < n_players; k++)
			if (scores[k] == best_score)
				ev[k] += delta_ev;
	}
	for (k = 0; k < n_players; k++)
		ev[k] /= (double)N;
	return 0;
}



int eval_monte_carlo_omaha(const int *HR, int N, int *board, int n_board, 
	int *pocket, int n_players, double *ev)
{
	const unsigned int pocket_perms[2][6] = {{0, 0, 0, 1, 1, 2}, {1, 2, 3, 2, 3, 3}};
	const unsigned int n_pocket_perms = 6;
	const unsigned int board_perms[10][3] = {
		{0, 1, 2}, // 3, 4, 5
		{0, 1, 3}, {0, 2, 3}, {1, 2, 3}, // 4, 5
		{0, 1, 4}, {0, 2, 4}, {0, 3, 4}, {1, 2, 4}, {1, 3, 4}, {2, 3, 4}}; // 5
	unsigned int n_board_perms = n_board == 5 ? 10 : n_board == 4 ? 4 : n_board == 3 ? 1 : -1;
	if (n_board_perms == -1)
		return 1;

	unsigned int mask[52], cards[52], n_cards = 0, n_mask = 0, n_available, 
		i, j, available_cards[52], sample[52], nb, np;

	memset(mask, 0, 52 * sizeof(unsigned int));
	memset(cards, 0, 52 * sizeof(unsigned int));
	memset(ev, 0, n_players * sizeof(double));
	uint64_t deck = new_deck();

	for (i = 0; i < n_board; i++)
	{
		if (!board[i])
			mask[n_mask++] = n_cards;
		else
			extract_cards(&deck, board[i]);
		cards[n_cards++] = board[i] + 1; // convert 0-51 to 1-52
	}
	for (i = 0; i < 4 * n_players; i++)
	{
		if (!pocket[i])
			mask[n_mask++] = n_cards;
		else
			extract_cards(&deck, pocket[i]);
		cards[n_cards++] = pocket[i] + 1; // convert 0-51 to 1-52
	}

	n_available = 52 - n_board - 4 * n_players + n_mask;
	get_cards(deck, available_cards, 1); // convert 0-51 to 1-52

	init_random_int_52();
	for (i = 0; i < N; ++i)
	{
		unsigned int best_score = 0;
		unsigned int tied = 0;
		random_sample_52_ross(n_available, n_mask, sample);
		for (j = 0; j < n_mask; ++j)
			cards[mask[j]] = available_cards[sample[j]];
		unsigned int *player_cards = cards + n_board;
		unsigned int board_paths[10];
		for (nb = 0; nb < n_board_perms; nb++)
			board_paths[nb] = HR[HR[HR[53 + 
				cards[board_perms[nb][0]]] + 
					cards[board_perms[nb][1]]] + 
						cards[board_perms[nb][2]]];
		unsigned int scores[MAX_PLAYERS];
		for (j = 0; j < n_players; ++j)
		{
			unsigned int score = 0;
			for (nb = 0; nb < n_board_perms; ++nb)
				for (np = 0; np < n_pocket_perms; ++np)
					// Aldanor: don't forget to use extra HR[...] for 5/6-card eval
					score = max(score, HR[HR[HR[board_paths[nb] + 
						player_cards[pocket_perms[0][np]]] + 
							player_cards[pocket_perms[1][np]]]]);
			scores[j] = score;
			if (score > best_score)
			{
				best_score = score;
				tied = 1;
			}
			else if (score == best_score)
				tied++;
			player_cards += 4;
		}
		double delta_ev = 1.0 / tied;
		for (j = 0; j < n_players; j++)
			if (scores[j] == best_score)
				ev[j] += delta_ev;
	}
	for (i = 0; i < n_players; ++i)
		ev[i] /= N;
	return 0;
}

void test_monte_carlo_1()
{
	static int HR[32487834]; 
	
	printf("\nLoading cards...\n");
	FILE *f = fopen("HandRanks.dat", "rb");
	fread(HR, sizeof(HR), 1, f);
	fclose(f);
	printf("Loading completed.\n");

	int board[5] = {0, 0, 0, 0, 0},
		n_board = 5,
		pocket[4] = {46, 30, 0, 0},
		n_players = 2;

	double ev[2];
	int N = 1e7;

	uint64_t start = mach_absolute_time();
	printf("\nGenerating %d Monte-Carlo hands...\n", N);
	eval_monte_carlo_holdem(HR, N, board, n_board, pocket, n_players, ev);
	double elapsed = get_time(mach_absolute_time(), start);
	printf("[HOLDEM] [Kh 9h] EV vs [* *] with board [* * * * *]: %.4f%%.\n", ev[0] * 100.);
	printf("Elapsed: %.4f seconds (%.0f hands / sec).\n", elapsed, (N / elapsed));	

	int omaha_board[5] = {2, 3, 0, 0, 0},
		omaha_n_board = 5,
		omaha_pocket[8] = {46, 30, 0, 0, 51, 0, 0, 0},
		omaha_n_players = 2;
	double omaha_ev[2];

	start = mach_absolute_time();
	printf("\nGenerating %d Monte-Carlo hands...\n", N);
	eval_monte_carlo_omaha(HR, N, omaha_board, omaha_n_board, omaha_pocket, 
		omaha_n_players, omaha_ev);
	elapsed = get_time(mach_absolute_time(), start);
	printf("[OMAHA] [Kh 9h * *] EV vs [As * * *] with board [2h 2s * * *]: %.4f%%.\n", omaha_ev[0] * 100.);
	printf("Elapsed: %.4f seconds (%.0f hands / sec).\n\n", elapsed, (N / elapsed));	

}

void test_sampling()
{
	const int N = 1e7, N2 = 1e6;
	const int n = 52;
	const int k = 10;
	int i, j, out[n];
	int counts[n];

	init_random_int_52();
	printf("Generating %d random samples (%d out of %d)...\n", N, k, n);
	uint64_t start = mach_absolute_time();
	for (i = 0; i < N; i++)
		random_sample_52_ross(n, k, out);
	double elapsed = get_time(mach_absolute_time(), start);

	printf("Verifying counts...\n");
	for (i = 0; i < N2; i++)
	{
		random_sample_52_ross(n, k, out);
		for (j = 0; j < k; j++)
			counts[out[j]]++;
	}

	for (i = 0; i < n; i++)
		printf("%6d: %.6f%%\n", i, 100.0 * ((double)counts[i] / k) / (double)N2);

	printf("\nRandom sampling took %.4f seconds (%.0f samples / sec).\n", elapsed, (N / elapsed));	
}

int main(void)
{
	test_monte_carlo_1();

	// test_sampling();

	return 0; 
}