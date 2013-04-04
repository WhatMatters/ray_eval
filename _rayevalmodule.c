#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <Python.h>
#include "arrays.h"
#include "load_file.h"

#define MAX_PLAYERS 		10

#define	STRAIGHT_FLUSH		1
#define	FOUR_OF_A_KIND		2
#define	FULL_HOUSE			3
#define	FLUSH				4
#define	STRAIGHT			5
#define	THREE_OF_A_KIND		6
#define	TWO_PAIR			7
#define	ONE_PAIR			8
#define	HIGH_CARD			9

static char *hand_rank_str[] = {
	"",
	"Straight Flush",
	"Four of a Kind",
	"Full House",
	"Flush",
	"Straight",
	"Three of a Kind",
	"Two Pair",
	"One Pair",
	"High Card"
};

const char HandRanks[][16] = {"BAD!!","High Card","Pair","Two Pair","Three of a Kind","Straight","Flush","Full House","Four of a Kind","Straight Flush"};


#define CLUB				0x8000
#define DIAMOND 			0x4000
#define HEART   			0x2000
#define SPADE   			0x1000

#define min(a, b)  			(((a) < (b)) ? (a) : (b))
#define max(a, b)  			(((a) > (b)) ? (a) : (b))

#define HR9_SIZE            347382723


////////////////////////////////////////////////////////////////////////////////
//							HAND RANKS .DAT FILE GENERATOR
////////////////////////////////////////////////////////////////////////////////

// perform a binary search on a pre-sorted array
int findit(int key)
{
    int low = 0, high = 4887, mid;
    while (low <= high)
    {
        mid = (high + low) >> 1; // divide by two
        if (key < products[mid])
            high = mid - 1;
        else if (key > products[mid])
            low = mid + 1;
        else
            return mid;
    }
    fprintf(stderr, "ERROR: no match found; key = %d\n", key);
    return -1;
}

//   This routine initializes the deck.  A deck of cards is
//   simply an integer array of length 52 (no jokers).  This
//   array is populated with each card, using the following
//   scheme:
//
//   An integer is made up of four bytes.  The high-order
//   bytes are used to hold the rank bit pattern, whereas
//   the low-order bytes hold the suit/rank/prime value
//   of the card.
//
//   +--------+--------+--------+--------+
//   |xxxbbbbb|bbbbbbbb|cdhsrrrr|xxpppppp|
//   +--------+--------+--------+--------+
//
//   p = prime number of rank (deuce=2, trey=3, four=5, five=7,..., ace=41)
//   r = rank of card (deuce=0, trey=1, four=2, five=3,..., ace=12)
//   cdhs = suit of card
//   b = bit turned on depending on rank of card
void init_deck(int *deck)
{
    int i, j, n = 0, suit = 0x8000;

    for (i = 0; i < 4; i++, suit >>= 1)
        for (j = 0; j < 13; j++, n++)
			deck[n] = primes[j] | (j << 8) | suit | (1 << (16 + j));
}

//  This routine will search a deck for a specific card
//  (specified by rank/suit), and return the INDEX giving
//  the position of the found card.  If it is not found,
//  then it returns -1
int find_card(int rank, int suit, int *deck)
{
    int i, c;
	for (i = 0; i < 52; i++)
	{
		c = deck[i];
		if ((c & suit) && (((c >> 8) & 0xF) == rank))
			return i;
	}
	return -1;
}

int hand_rank(short val)
{
    if (val > 6185) return HIGH_CARD;        	// 1277 high card
    if (val > 3325) return ONE_PAIR;         	// 2860 one pair
    if (val > 2467) return TWO_PAIR;         	//  858 two pair
    if (val > 1609) return THREE_OF_A_KIND;  	//  858 three-kind
    if (val > 1599) return STRAIGHT;         	//   10 straights
    if (val > 322)  return FLUSH;            	// 1277 flushes
    if (val > 166)  return FULL_HOUSE;       	//  156 full house
    if (val > 10)   return FOUR_OF_A_KIND;   	//  156 four-kind
    return STRAIGHT_FLUSH;                  	//   10 straight-flushes
}

short eval_5cards(int c1, int c2, int c3, int c4, int c5)
{
    int q;
    short s;
    q = (c1 | c2 | c3 | c4 | c5) >> 16;
    if (c1 & c2 & c3 & c4 & c5 & 0xF000) // check for flushes and straight-flushes
		return( flushes[q] );
    s = unique5[q]; // check for straights and high-card hands
    if (s) 
    	return s;
    q = (c1 & 0xFF) * (c2 & 0xFF) * (c3 & 0xFF) * (c4 & 0xFF) * (c5 & 0xFF);
    q = findit(q);
    return values[q];
}

short eval_5hand(int *hand)
{
    int c1, c2, c3, c4, c5;
    c1 = *hand++; c2 = *hand++; c3 = *hand++; c4 = *hand++; c5 = *hand;
    return eval_5cards(c1, c2, c3, c4, c5);
}

short eval_7hand(int *hand)
{
    int i, j, q, best = 9999, subhand[5];
	for (i = 0; i < 21; i++)
	{
		for (j = 0; j < 5; j++)
			subhand[j] = hand[perm7[i][j]];
		q = eval_5hand(subhand);
		if (q < best)
			best = q;
	}
	return best;
}

int n_cards = 0;

// add a new card to this id
int64_t make_id(int64_t id_in, int new_card) 
{
	int n_suit[4 + 1], n_rank[13 + 1], n, done = 0, needsuited;
	int cards[8];  // intentially keep an extra one as 0 end
	
	memset(cards, 0, sizeof(cards));
	memset(n_rank, 0, sizeof(n_rank));
	memset(n_suit, 0, sizeof(n_suit));
	
	for (n = 0; n < 6; n++) // can't have more than 6 cards
		cards[n + 1] = (int) ((id_in >> (8 * n)) & 0xff);  // leave the 0 index for new card

	new_card--;  // 1-52 -> 0-51
	cards[0] = (((new_card >> 2) + 1) << 4) + (new_card & 3) + 1;  
	// add next card formats card to rrrr00ss

	for (n_cards = 0; cards[n_cards]; n_cards++) 
	{
		n_suit[cards[n_cards] & 0xf]++; // need to see if suit is significant
		n_rank[(cards[n_cards] >> 4) & 0xf]++; // and rank to be sure we don't have 4
		if (n_cards && (cards[0] == cards[n_cards])) // can't have the same card twice
			done = 1; // if so - need to quit after counting n_cards
	}

	if (done)
		return 0; // duplicate of another card (ignore this one)    
	
	needsuited = n_cards - 2; // for suit to be significant - need to have n-2 of same suit
	     
	int rank;
	if (n_cards > 4)  
		for (rank = 1; rank < 14; rank++)
			if (n_rank[rank] > 4) // >= 4 of a rank => shouldn't do this one
				return 0; // can't have > 4 of a rank so return an invalid id
	
	// In the ID process, if we have
	// 2s = 0x21, 3s = 0x31,.... Kc = 0xD4, Ac = 0xE4
	// then it allows us to sort in rank then suit order
	
	// if we don't have at least 2 cards of the same suit for 4, we make this card suit 0.
	
	if (needsuited > 1)
		for (n = 0; n < n_cards; n++) // for each card
			if (n_suit[cards[n] & 0xf] < needsuited) // check n_suit to the number I need to have suits significant
				cards[n] &= 0xf0; // if not enough - 0 out the suit - now this suit would be a 0 vs 1-4

	// sort using XOR (network for N = 7, using Bose-Nelson algorithm)
	#define SWAP(I, J) {if (cards[I] < cards[J]) {cards[I] ^= cards[J]; cards[J] ^= cards[I]; cards[I] ^= cards[J];}}		

	SWAP(0, 4);	SWAP(1, 5);	SWAP(2, 6);	SWAP(0, 2);		
	SWAP(1, 3);	SWAP(4, 6);	SWAP(2, 4);	SWAP(3, 5);		
	SWAP(0, 1);	SWAP(2, 3);	SWAP(4, 5);	SWAP(1, 4);		
	SWAP(3, 6);	SWAP(1, 2);	SWAP(3, 4);	SWAP(5, 6);	

	// store cards in bytes --66554433221100	 
	// the resulting ID is a 64 bit value with each card represented by 8 bits.
	return ((int64_t) cards[0] +
		 ((int64_t) cards[1] << 8) +
		 ((int64_t) cards[2] << 16) + 
		 ((int64_t) cards[3] << 24) +
		 ((int64_t) cards[4] << 32) +
		 ((int64_t) cards[5] << 40) +
		 ((int64_t) cards[6] << 48));    
}

int save_id(int64_t id, int64_t *ids, int64_t *max_id, int *num_ids) 
{
	if (id == 0) // don't use up a record for 0
		return 0; 

	if (id >= (*max_id)) // take care of the most likely first goes on the end...
	{           		
		if (id > (*max_id)) // greater than create new else it was the last one!
		{        
			ids[(*num_ids)++] = id; // add the new id
			(*max_id) = id;
		}
		return (*num_ids) - 1;
	}

	// pseudo bsearch algorithm
	int holdtest, low = 0, high = (*num_ids) - 1;
	int64_t testval;

	while (high - low > 1) 
	{
		holdtest = (high + low + 1) / 2;
		testval = ids[holdtest] - id;
		if (testval > 0) 
			high = holdtest;
		else if (testval < 0) 
			low = holdtest;
		else 
			return holdtest; // got it
	}
	// I guess it couldn't be found so must be added to the current location (high)
	// make space...  // don't expect this much!
	memmove(&ids[high + 1], &ids[high], ((*num_ids) - high) * sizeof(ids[0]));  

	ids[high] = id;   // do the insert into the hole created
	(*num_ids)++;        
	return high;
}

int do_eval(int64_t id_in)
{
	int n, handrank = 0, wcard, rank, suit, suititerator = 0, holdrank;
	int mainsuit = 20; // just something that will never hit...  need to eliminate the main suit from the iterator
	int wcards[8];  // work cards, intentially keep one as a 0 end
	int holdcards[8];
	int numevalcards = 0;

	// See Cactus Kevs page for explainations for this type of stuff...
	const int primes[] = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41};
	
	memset(wcards, 0, sizeof(wcards));
	memset(holdcards, 0, sizeof(holdcards));

	if (id_in) // if then id is good then do it
	{
		for (n = 0; n < 7; n++) // convert all 7 cards (0s are ok)
		{  
			holdcards[n] =  (int) ((id_in >> (8 * n)) & 0xff); 
			if (holdcards[n] == 0) 
				break;	// once I hit a 0 I know I am done
			numevalcards++;						// if not 0 then count the card
			if ((suit = holdcards[n] & 0xf))	// find out what suit (if any) was significant
				mainsuit = suit;					// and remember it
		}

		for (n = 0; n < numevalcards; n++) // just have n_cards...
		{  
			wcard = holdcards[n];

			// convert to cactus kevs way!!  ref http://www.suffecool.net/poker/evaluator.html
			//   +--------+--------+--------+--------+
			//   |xxxbbbbb|bbbbbbbb|cdhsrrrr|xxpppppp|
			//   +--------+--------+--------+--------+
			//   p = prime number of rank (deuce=2,trey=3,four=5,five=7,...,ace=41)
			//   r = rank of card (deuce=0,trey=1,four=2,five=3,...,ace=12)
			//   cdhs = suit of card
			//   b = bit turned on depending on rank of card

			rank = (wcard >> 4) - 1;	 // my rank is top 4 bits 1-13 so convert
			suit = wcard & 0xf;  // my suit is bottom 4 bits 1-4, order is different, but who cares?  
			if (suit == 0) // if suit wasn't significant though...
			{		
				suit = suititerator++;   // Cactus Kev needs a suit!
				if (suititerator == 5)	 // loop through available suits
					suititerator = 1;
				if (suit == mainsuit) // if it was the sigificant suit...  Don't want extras!!
				{   
					suit = suititerator++;    // skip it
					if (suititerator == 5)	  // roll 1-4
						suititerator = 1;
				}
			}
			// now make Cactus Kev card
			wcards[n] = primes[rank] | (rank << 8) | (1 << (suit + 11)) | (1 << (16 + rank));
		}

		switch (numevalcards) // run Cactus Keys routines
		{  
			case 5:  
				holdrank = eval_5cards(wcards[0],wcards[1],wcards[2],wcards[3],wcards[4]);
				break;
			// if 6 cards I would like to find HandRank for them 
			// Cactus Key is 1 = highest - 7362 lowest I need to get the min for the permutations
			case 6:  
				holdrank = eval_5cards(wcards[0], wcards[1], wcards[2], wcards[3], wcards[4]);
				holdrank = min(holdrank, eval_5cards(wcards[0], wcards[1], wcards[2], wcards[3], wcards[5]));
				holdrank = min(holdrank, eval_5cards(wcards[0], wcards[1], wcards[2], wcards[4], wcards[5]));
				holdrank = min(holdrank, eval_5cards(wcards[0], wcards[1], wcards[3], wcards[4], wcards[5]));
				holdrank = min(holdrank, eval_5cards(wcards[0], wcards[2], wcards[3], wcards[4], wcards[5]));
				holdrank = min(holdrank, eval_5cards(wcards[1], wcards[2], wcards[3], wcards[4], wcards[5]));
				break;
			case 7: 
				holdrank = eval_7hand(wcards);  
				break;
			default : // problem!!  shouldn't hit this... 
				return -1;
		}

		// change the format of Catus Kev's ret value to:
		// hhhhrrrrrrrrrrrr   hhhh = 1 high card -> 9 straight flush
		//                    r..r = rank within the above	1 to max of 2861
		handrank = 7463 - holdrank;  // now the worst hand = 1
		
		if      (handrank < 1278) handrank = handrank -    0 + 4096 * 1;  // 1277 high card
		else if (handrank < 4138) handrank = handrank - 1277 + 4096 * 2;  // 2860 one pair
		else if (handrank < 4996) handrank = handrank - 4137 + 4096 * 3;  //  858 two pair
		else if (handrank < 5854) handrank = handrank - 4995 + 4096 * 4;  //  858 three-kind
		else if (handrank < 5864) handrank = handrank - 5853 + 4096 * 5;  //   10 straights
		else if (handrank < 7141) handrank = handrank - 5863 + 4096 * 6;  // 1277 flushes
		else if (handrank < 7297) handrank = handrank - 7140 + 4096 * 7;  //  156 full house
		else if (handrank < 7453) handrank = handrank - 7296 + 4096 * 8;  //  156 four-kind
		else                      handrank = handrank - 7452 + 4096 * 9;  //   10 straight-flushes

	}
	return handrank;
}

int generate_handranks(const char *filename)
{
	int card, count = 0, num_ids = 1, n, id_slot, max_handrank = 0, handTypeSum[10], holdid;
	int64_t ID, max_id = 0LLU;
	int *_HR = malloc(32487834 * sizeof(int));
	int64_t *ids = malloc(612978 * sizeof(int64_t));

	memset(handTypeSum, 0, sizeof(handTypeSum));  // init
	memset((long long *) ids, 0LLU, sizeof(ids));
	memset((int *) _HR, 0, sizeof(_HR));

	// as this loops through and find new combinations it adds them to the end
	// I need this list to be stable when I set the handranks (next set)  (I do the insertion sort on new IDs these)
	// so I had to get the IDs first and then set the handranks
	for (n = 0; ids[n] || n == 0; n++) // start at 1 so I have a zero catching entry (just in case)
	{  
		for (card = 1; card < 53; card++) // the ids above contain cards upto the current card.  Now add a new card 
		{	 
			ID = make_id(ids[n], card);   // get the new ID for it
			if (n_cards < 7) 
				holdid = save_id(ID, ids, &max_id, &num_ids);   // and save it in the list if I am not on the 7th card
		}
		if ((n + 2) % 19 == 0)
			printf("\rGenerating card ids... %6d / 612977", n + 1);	  // just to show the progress -- this will count up to  612976
	}
	printf("\n");
	// this is as above, but will not be adding anything to the ID list, so it is stable
	for (n = 0; ids[n] || n == 0; n++) // start at 1 so I have a zero catching entry (just in case)
	{  
		for (card = 1; card < 53; card++) {
			ID = make_id(ids[n], card);
			if (n_cards < 7)
				id_slot = save_id(ID, ids, &max_id, &num_ids) * 53 + 53;  // when in the index mode (< 7 cards) get the id to save
			else
				id_slot = do_eval(ID);   // if I am at the 7th card, get the HandRank to save
			if (id_slot == -1)
			{
				printf("    Error: problem with n_cards = %d.\n", n_cards);
				free(_HR); free(ids); _HR = 0; ids = 0; return 1;
			}
			max_handrank = n * 53 + card + 53;	// find where to put it 
			_HR[max_handrank] = id_slot;				// and save the pointer to the next card or the handrank
		}

		if (n_cards == 6 || n_cards == 7) 
		{  
			// an extra, If you want to know what the handrank when there is 5 or 6 cards
			// you can just do HR[u3] or HR[u4] from below code for Handrank of the 5 or 6 card hand
			int value = do_eval(ids[n]);
			if (value == -1)
			{
				printf("    Error: problem with n_cards = %d.\n", n_cards);
				free(_HR); free(ids); _HR = 0; ids = 0; return 1;
			}
			_HR[n * 53 + 53] = value;  // this puts the above handrank into the array  
		}
		if ((n + 2) % 19 == 0)
			printf("\rSetting hand ranks...  %6d / 612977", n + 1);	  // just to show the progress -- this will count up to  612976
	}

	printf("\nThe highest hand rank: %d.\n", max_handrank);

	int c0, c1, c2, c3, c4, c5, c6;
	int u0, u1, u2, u3, u4, u5;

	for (c0 = 1; c0 < 53; c0++) {
		u0 = _HR[53+c0];
		for (c1 = c0+1; c1 < 53; c1++) {
			u1 = _HR[u0+c1];
			for (c2 = c1+1; c2 < 53; c2++) {
				u2 = _HR[u1+c2];
				for (c3 = c2+1; c3 < 53; c3++) {
					u3 = _HR[u2+c3];
					for (c4 = c3+1; c4 < 53; c4++) {
						u4 = _HR[u3+c4];
						for (c5 = c4+1; c5 < 53; c5++) {
							u5 = _HR[u4+c5];
							for (c6 = c5+1; c6 < 53; c6++) {
								handTypeSum[_HR[u5+c6] >> 12]++;
								count++;
							}
						}
					}
				}
			}
		}
	}

	int i;
	for (i = 0; i <= 9; i++)
		printf("\n%16s: %d", HandRanks[i], handTypeSum[i]);
	
	printf("\nTotal hands = %d\n", count);

	FILE * fout = fopen(filename, "wb");
	int result = 1;
	if (fout)
	{
		result = 0;
        fwrite(_HR, sizeof(int), 32487834, fout);
		fclose(fout);
	}
	free(_HR); free(ids); _HR = 0; ids = 0;
	return result;
}

////////////////////////////////////////////////////////////////////////////////
//							UTILITY STUFF
////////////////////////////////////////////////////////////////////////////////

static int *HR = 0;

int *load_handranks(const char *filename)
{
	static int _HR[32487834];
	FILE *f = fopen(filename, "rb");
	if (f)
	{
		load_file((char *)_HR, sizeof(_HR), 1, f);
		fclose(f);
		return _HR;
	}
	else
		return NULL;
}

int *HR9 = 0;

int *load_handranks_9(const char *filename)
{
	FILE *f = fopen(filename, "rb");
	if (f)
	{
		int size = 0;
		fread(&size, sizeof(int), 1, f);
    	int *_HR9 = (int *) malloc(size * sizeof(int));
		load_file((char *)_HR9, sizeof(int), size, f);
		fclose(f);
		return _HR9;
	}
	else
		return NULL;
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

void init_random_int_52(void)
{
	int i;
	for (i = 1; i <= 52; i++)
		RAND_MAX_DIV_52[i] = RAND_MAX / (i + 1);
	srand((unsigned int) mix(clock(), time(NULL), getpid()));
}

int random_int_52(int k) 
{
    int r;
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

void extract_cards(uint64_t *deck, int card)
{
	*deck ^= (1LLU << card);
}

void get_cards(uint64_t deck, int *cards, int shift)
{
	int i, pos = 0;
	for (i = 0; i < 52; i++)
		if (deck & (1LLU << i))
			cards[pos++] = i + shift;
}

uint64_t new_deck()
{
	uint64_t deck = 0;
	int i;
	for (i = 0; i < 52; i++)
		deck |= (1LLU << i);
	return deck;
}

////////////////////////////////////////////////////////////////////////////////
//							MONTE CARLO EVAL
////////////////////////////////////////////////////////////////////////////////


int eval_monte_carlo_holdem(int N, int *board, int n_board, 
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
		if (board[i] == 255)
			mask[n_mask++] = n_cards;
		else
			extract_cards(&deck, board[i]);
		cards[n_cards++] = board[i] + 1; // convert 0-51 to 1-52
	}
	for (i = 0; i < 2 * n_players; i++)
	{
		if (pocket[i] == 255)
			mask[n_mask++] = n_cards;
		else
			extract_cards(&deck, pocket[i]);
		cards[n_cards++] = pocket[i] + 1; // convert 0-51 to 1-52
	}
	n_available = 52 - n_board - 2 * n_players + n_mask;
	get_cards(deck, available_cards, 1); // convert 0-51 to 1-52
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

int eval_monte_carlo_omaha_9(int N, int *board, int n_board, 
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
	int fs_offset = (n_board == 5) ? 106 : ((n_board == 4) ? HR9[106] : HR9[HR9[106]]);
	int snf_offset = (n_board == 5) ? (HR9[0] + 53) : 
		((n_board == 4) ? HR9[HR9[0] + 53] : HR9[HR9[HR9[0] + 53]]);
	int flush_offset = (n_board == 5) ? (HR9[1] + 56) : 
		((n_board == 4) ? HR9[HR9[1] + 56] : HR9[HR9[HR9[1] + 56]]);
	for (i = 0; i < n_board; i++)
	{
		if (board[i] == 255)
			mask[n_mask++] = n_cards;
		else
			extract_cards(&deck, board[i]);
		cards[n_cards++] = board[i] + 1; // convert 0-51 to 1-52
	}
	for (i = 0; i < 4 * n_players; i++)
	{
		if (pocket[i] == 255)
			mask[n_mask++] = n_cards;
		else
			extract_cards(&deck, pocket[i]);
		cards[n_cards++] = pocket[i] + 1; // convert 0-51 to 1-52
	}
	n_available = 52 - n_board - 4 * n_players + n_mask;
	get_cards(deck, available_cards, 1); // convert 0-51 to 1-52
	int *HR9_f;
	for (i = 0; i < N; i++)
	{
		int sample[52], scores[MAX_PLAYERS], best_score = -1, tied = 0;
		random_sample_52_ross(n_available, n_mask, sample);
		for (j = 0; j < n_mask; j++)
			cards[mask[j]] = available_cards[sample[j]];
		int board_fs = fs_offset;
		int board_snf = snf_offset;
		for (j = 0; j < n_board; j++)
		{
			board_fs = HR9[board_fs + cards[j]];
			board_snf = HR9[board_snf + cards[j]];
		}
		int *player_cards = cards + n_board;
		for (k = 0; k < n_players; k++)
		{
			int fs = board_fs;
			int score = board_snf;
			for (j = 0; j < 4; j++)
			{
				fs = HR9[fs + player_cards[j]];
				score = HR9[score + player_cards[j]];
			}
			if (fs != 0)
			{
				HR9_f = HR9 + (4 - fs);
				int sf = flush_offset;
				for (j = 0; j < n_board; j++)
					sf = HR9_f[sf + cards[j]];
				for (j = 0; j < 4; j++)
					sf = HR9_f[sf + player_cards[j]];
				score = max(score, sf);
			}
			
			scores[k] = score;
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
		for (k = 0; k < n_players; k++)
			if (scores[k] == best_score)
				ev[k] += delta_ev;
	}
	for (k = 0; k < n_players; k++)
		ev[k] /= (double)N;
	return 0;
}

static int pocket_perms[2][6] = {{0, 0, 0, 1, 1, 2}, {1, 2, 3, 2, 3, 3}};
static int n_pocket_perms = 6;
static int board_perms[10][3] = {
	{0, 1, 2}, // 3, 4, 5
	{0, 1, 3}, {0, 2, 3}, {1, 2, 3}, // 4, 5
	{0, 1, 4}, {0, 2, 4}, {0, 3, 4}, {1, 2, 4}, {1, 3, 4}, {2, 3, 4}}; // 5

int eval_monte_carlo_omaha(int N, int *board, int n_board, 
	int *pocket, int n_players, double *ev)
{
	int n_board_perms = n_board == 5 ? 10 : n_board == 4 ? 4 : n_board == 3 ? 1 : -1;
	if (n_board_perms == -1)
		return 1;

	int mask[52], cards[52], n_mask = 0, n_available, 
		i, available_cards[52], sample[52];

	memset(mask, 0, 52 * sizeof(int));
	memset(cards, 0, 52 * sizeof(int));
	memset(ev, 0, n_players * sizeof(double));
	uint64_t deck = new_deck();

	for (i = 0; i < n_board; i++)
	{
		if (board[i] == 255)
			mask[n_mask++] = i;
		else
			extract_cards(&deck, board[i]);
		cards[i] = board[i] + 1; // convert 0-51 to 1-52
	}
	for (i = 0; i < 4 * n_players; i++)
	{
		if (pocket[i] == 255)
			mask[n_mask++] = n_board + i;
		else
			extract_cards(&deck, pocket[i]);
		cards[n_board + i] = pocket[i] + 1; // convert 0-51 to 1-52
	}

	n_available = 52 - n_board - 4 * n_players + n_mask;
	get_cards(deck, available_cards, 1); // convert 0-51 to 1-52

	for (i = 0; i < N; ++i)
	{
		int best_score = 0;
		int tied = 0;
		random_sample_52_ross(n_available, n_mask, sample);
		int j = 0, nb = 0, np = 0;
		for (j = 0; j < n_mask; ++j)
			cards[mask[j]] = available_cards[sample[j]];
		int *player_cards = cards + n_board;
		int board_paths[10];
		for (nb = 0; nb < n_board_perms; nb++)
			board_paths[nb] = HR[HR[HR[53 + 
				cards[board_perms[nb][0]]] + 
					cards[board_perms[nb][1]]] + 
						cards[board_perms[nb][2]]];
		int scores[MAX_PLAYERS];
		for (j = 0; j < n_players; ++j)
		{
			int score = 0;
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

////////////////////////////////////////////////////////////////////////////////
//							PYTHON WRAPPERS
////////////////////////////////////////////////////////////////////////////////

#define RAISE_EXCEPTION(exception, message) {PyErr_SetString((exception), (message)); return NULL;}

static PyObject *_rayeval_seed(PyObject *self, PyObject *args)
{
	unsigned int seed;
  	if (!PyArg_ParseTuple(args, "i", &seed))
    	return NULL;
    srand(seed);
	Py_RETURN_NONE;
}


static PyObject *_rayeval_load_handranks(PyObject *self, PyObject *args)
{
	char *filename;
  	if (!PyArg_ParseTuple(args, "s", &filename))
    	return NULL;
    if (!HR)
	    if (!(HR = load_handranks(filename)))
	    	RAISE_EXCEPTION(PyExc_RuntimeError, "Failed to load hand ranks from file.");
	Py_RETURN_NONE;
}

static PyObject *_rayeval_load_handranks_9(PyObject *self, PyObject *args)
{
	char *filename;
  	if (!PyArg_ParseTuple(args, "s", &filename))
    	return NULL;
    if (!HR9)
	    if (!(HR9 = load_handranks_9(filename)))
	    	RAISE_EXCEPTION(PyExc_RuntimeError, "Failed to load hand ranks [9] from file.");
	Py_RETURN_NONE;
}

static PyObject *_rayeval_generate_handranks(PyObject *self, PyObject *args)
{
	char *filename;
  	if (!PyArg_ParseTuple(args, "s", &filename))
    	return NULL;
	if (generate_handranks(filename))
		RAISE_EXCEPTION(PyExc_RuntimeError, "Failed to generate hand ranks file.");
	Py_RETURN_NONE;
}


int *parse_board_and_pockets(char *game, PyObject *py_board, PyObject *py_pocket,
	int *board, int *pocket, int *n_board, int *n_pocket, int *n_players, int *is_omaha,
	int *is_omaha_9)
{
	int i, pocket_size = 2;
	static int ok = 1;
	*is_omaha = 0;
	*is_omaha_9 = 0;

	if (!strcmp(game, "omaha"))
		*is_omaha = 1;
	else if (!strcmp(game, "omaha_9"))
	{
		*is_omaha = 1;
		*is_omaha_9 = 1;
	}
	else if (strcmp(game, "holdem"))
    	RAISE_EXCEPTION(PyExc_ValueError, "Game type must be holdem or omaha.");
    pocket_size += 2 * (*is_omaha);

  	if (!PyList_Check(py_board)) 
    	RAISE_EXCEPTION(PyExc_TypeError, "Board must be a list.");
  	if (!PyList_Check(py_pocket)) 
    	RAISE_EXCEPTION(PyExc_TypeError, "Pockets must be a list.");

    *n_pocket = (int) PyList_Size(py_pocket);
    *n_board = (int) PyList_Size(py_board);
    *n_players = *n_pocket / pocket_size;
    if ((*n_players < 1) || (*n_players > MAX_PLAYERS))
    	RAISE_EXCEPTION(PyExc_ValueError, "Invalid number of players.");
    if ((*n_players) * pocket_size != (*n_pocket))
    	RAISE_EXCEPTION(PyExc_ValueError, "Invalid number of pocket cards.");
    if ((*n_board != 3) && (*n_board != 4) && (*n_board != 5))
    	RAISE_EXCEPTION(PyExc_ValueError, "Board must contain 3-5 cards.");

    for (i = 0; i < (*n_pocket); i++)
    {
    	PyObject *item = PyList_GetItem(py_pocket, i);
    	if (!PyInt_Check(item))
	    	RAISE_EXCEPTION(PyExc_TypeError, "Pocket cards must be integers.");
    	pocket[i] = (int) PyInt_AsLong(item);
    	if ((pocket[i] < 0 || pocket[i] > 51) && pocket[i] != 255)
	    	RAISE_EXCEPTION(PyExc_TypeError, "Pocket cards must be 0-51 or 255.");
    }

    for (i = 0; i < (*n_board); i++)
    {
    	PyObject *item = PyList_GetItem(py_board, i);
    	if (!PyInt_Check(item))
	    	RAISE_EXCEPTION(PyExc_TypeError, "Board cards must be integers.");
    	board[i] = (int) PyInt_AsLong(item);
    	if ((board[i] < 0 || board[i] > 51) && board[i] != 255)
	    	RAISE_EXCEPTION(PyExc_TypeError, "Board cards must be 0-51 or 255.");
    }

    return &ok;
}

static PyObject *_rayeval_eval_hand(PyObject *self, PyObject *args)
{
	if (!HR)
		RAISE_EXCEPTION(PyExc_RuntimeError, "Please call load_handranks() first.");	

	char *game;
	PyObject *py_board, *py_pocket;
	int n_board, n_pocket, i, value;
	int n_players, board[5], pocket[4 * MAX_PLAYERS], is_omaha, is_omaha_9;

	if (!PyArg_ParseTuple(args, "sOO", &game, &py_board, &py_pocket))
		return NULL;

	if (!parse_board_and_pockets(game, py_board, py_pocket, board, pocket, 
		&n_board, &n_pocket, &n_players, &is_omaha, &is_omaha_9))
		return NULL;

	if (n_players > 1)
		RAISE_EXCEPTION(PyExc_ValueError, "One player expected, found more.");

	for (i = 0; i < n_board; i++)
		if (board[i] == 255)
			RAISE_EXCEPTION(PyExc_ValueError, "Masked board is not allowed.");
	for (i = 0; i < n_pocket; i++)
		if (pocket[i] == 255)
			RAISE_EXCEPTION(PyExc_ValueError, "Masked pocket is not allowed.");

	if (is_omaha_9)
	{
		int fs = 106, snf = HR9[0] + 53, fo = HR9[1] + 56;
		if (n_board < 5) { fs = HR9[fs]; snf = HR9[snf]; fo = HR9[fo]; }
		if (n_board < 4) { fs = HR9[fs]; snf = HR9[snf]; fo = HR9[fo]; }
		for (i = 0; i < n_board; i++)
		{
			fs = HR9[fs + board[i] + 1]; 
			snf = HR9[snf + board[i] + 1];
		}
		for (i = 0; i < n_pocket; i++)
		{
			fs = HR9[fs + pocket[i] + 1];
			snf = HR9[snf + pocket[i] + 1];
		}
		value = snf;
		if (fs != 0)
		{
			int *HR9_f = HR9 + (4 - fs);
			int flush_score = fo;
			for (i = 0; i < n_board; i++)
				flush_score = HR9_f[flush_score + board[i] + 1]; 
			for (i = 0; i < n_pocket; i++)
				flush_score = HR9_f[flush_score + pocket[i] + 1];
			value = (flush_score > value) ? flush_score : value;
		}
		return PyInt_FromLong((long)value);
	}
	else if (is_omaha)
	{
		int n_board_perms = n_board == 5 ? 10 : n_board == 4 ? 4 : n_board == 3 ? 1 : -1;
		int board_paths[10], nb, np;
		for (nb = 0; nb < n_board_perms; nb++)
			board_paths[nb] = HR[HR[HR[53 + 
				board[board_perms[nb][0]] + 1] + 
					board[board_perms[nb][1]] + 1] + 
						board[board_perms[nb][2]] + 1];
		value = 0;
		for (nb = 0; nb < n_board_perms; ++nb)
			for (np = 0; np < n_pocket_perms; ++np)
				value = max(value, HR[HR[HR[board_paths[nb] + 
						pocket[pocket_perms[0][np]] + 1] + 
							pocket[pocket_perms[1][np]] + 1]]);
		return PyInt_FromLong((long)value);
	}
	else
	{
		value = 53;
		for (i = 0; i < n_board; i++)
			value = HR[value + board[i] + 1];
		for (i = 0; i < n_pocket; i++)
			value = HR[value + pocket[i] + 1];
		if (n_board + n_pocket < 7)
			value = HR[value];
		return PyInt_FromLong((long)value);
	}
}

/*
INPUT:
	game: "omaha" | "holdem"
	board: list (int)
	pockets: list (int)
	iterations: int
OUTPUT:
	ev: list (doble)
*/
static PyObject *_rayeval_eval_mc(PyObject *self, PyObject *args)
{
	if (!HR)
		RAISE_EXCEPTION(PyExc_RuntimeError, "Please call load_handranks() first.");

	char *game;
	PyObject *py_board, *py_pocket, *py_ev;
	int i, n_board, n_pocket;
	int iterations, n_players, board[5], pocket[4 * MAX_PLAYERS], is_omaha, is_omaha_9;
	double ev[MAX_PLAYERS];

	if (!PyArg_ParseTuple(args, "sOOi", &game, &py_board, &py_pocket, &iterations))
		return NULL;

    if (iterations <= 0)
    	RAISE_EXCEPTION(PyExc_ValueError, "Iterations must be a positive integer.");

	if (!parse_board_and_pockets(game, py_board, py_pocket, board, pocket, 
		&n_board, &n_pocket, &n_players, &is_omaha, &is_omaha_9))
		return NULL;

	if (is_omaha_9)
   		eval_monte_carlo_omaha_9(iterations, board, n_board, pocket, n_players, ev);
   	else if (is_omaha)
   		eval_monte_carlo_omaha(iterations, board, n_board, pocket, n_players, ev);
   	else
   		eval_monte_carlo_holdem(iterations, board, n_board, pocket, n_players, ev);

   	py_ev = PyList_New(n_players);
   	for (i = 0; i < n_players; i++)
   		PyList_SET_ITEM(py_ev, (Py_ssize_t) i, PyFloat_FromDouble(ev[i]));

   	return py_ev;
}

////////////////////////////////////////////////////////////////////////////////
//							MODULE INITIALIZATION
////////////////////////////////////////////////////////////////////////////////

static PyObject *_rayeval_test(PyObject *self, PyObject *args)
{
	Py_RETURN_NONE;
}

static PyMethodDef _rayeval_methods[] = {
	{"seed", (PyCFunction) _rayeval_seed, METH_VARARGS, ""},
	{"generate_handranks", (PyCFunction) _rayeval_generate_handranks, METH_VARARGS, ""},
	{"load_handranks", (PyCFunction) _rayeval_load_handranks, METH_VARARGS, ""},
	{"load_handranks_9", (PyCFunction) _rayeval_load_handranks_9, METH_VARARGS, ""},
	{"eval_mc", (PyCFunction) _rayeval_eval_mc, METH_VARARGS, ""},
	{"eval_hand", (PyCFunction) _rayeval_eval_hand, METH_VARARGS, ""},
	{"test", (PyCFunction) _rayeval_test, METH_NOARGS, ""},
	{NULL, NULL, 0, NULL}
};

PyMODINIT_FUNC init_rayeval(void)
{
	init_random_int_52();
	Py_InitModule("_rayeval", _rayeval_methods);
}
