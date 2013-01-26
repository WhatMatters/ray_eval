#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mach/mach.h>
#include <mach/mach_time.h>

// recommended compile flags: -O3 -msse4 -fPIC -m64

const uint64_t ONE_64 = 1LLU;

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


inline void extract_cards(uint64_t *deck, int card)
{
	*deck ^= (ONE_64 << card);
}

inline void get_cards(uint64_t deck, int *cards, int shift)
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

int main(void)
{
	static int HR[32487834]; 
	
	printf("Loading cards...\n");
	FILE *f = fopen("HandRanks.dat", "rb");
	fread(HR, sizeof(HR), 1, f);
	fclose(f);
	printf("Loading completed.\n");

	uint64_t start = mach_absolute_time();
	uint64_t deck = new_deck(), deck2;

	int pocket_card_1 = 46, pocket_card_2 = 30; // Kh, 9h
	extract_cards(&deck, pocket_card_1);
	extract_cards(&deck, pocket_card_2);
	
	int unknown_cards[50], remaining_cards[45];
	get_cards(deck, unknown_cards, 0);

	long equity_count = 0;
	int i, b0, b1, b2, b3, b4, c0, c1, c2, c3, c4, u0, u1, u2, u3, u4, o1, o2, u5;
	int hero, villain;

	for (c0 = 0; c0 < 50; c0++)  
	{
		b0 = unknown_cards[c0];
		u0 = HR[53 + b0 + 1];
		for (c1 = c0 + 1; c1 < 50; c1++) 
		{
			b1 = unknown_cards[c1];
			u1 = HR[u0 + b1 + 1];
			for (c2 = c1 + 1; c2 < 50; c2++) 
			{
				b2 = unknown_cards[c2];
				u2 = HR[u1 + b2 + 1];
				for (c3 = c2 + 1; c3 < 50; c3++) 
				{
					b3 = unknown_cards[c3];
					u3 = HR[u2 + b3 + 1];
					for (c4 = c3 + 1; c4 < 50; c4++) 
					{
						b4 = unknown_cards[c4];
						u4 = HR[u3 + b4 + 1];
						deck2 = deck;
						extract_cards(&deck2, b0);
						extract_cards(&deck2, b1);
						extract_cards(&deck2, b2);
						extract_cards(&deck2, b3);
						extract_cards(&deck2, b4);
						get_cards(deck2, remaining_cards, 1);
						hero = HR[HR[u4 + pocket_card_1 + 1] + pocket_card_2 + 1];
						for (o1 = 0; o1 < 45; o1++)
						{
							u5 = HR[u4 + remaining_cards[o1]];
							for (o2 = o1 + 1; o2 < 45; o2++)
							{
								villain = HR[u5 + remaining_cards[o2]];
								if (hero > villain)
									equity_count += 2;
								if (hero == villain)
									equity_count += 1;
							}
						}
					}
				}
			}
		}
	}	

	double ev = (double)equity_count / (2118760.0 * 990.0 * 2.0);
	printf("\nElapsed: %.4f seconds.\n", get_time(mach_absolute_time(), start));
	printf("Kh9h equity against a random hand is: %.6f.\n", ev);
	return 0; 
}