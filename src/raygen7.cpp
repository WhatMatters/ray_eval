#include <iostream>
#include <iomanip>
#include <string.h>
#include <stdlib.h>

#include "arrays.h"
#include "rayutils.h"

void init_deck(int *deck)
{
    int n = 0, suit = 0x8000;
    for (int i = 0; i < 4; i++, suit >>= 1)
        for (int j = 0; j < 13; j++, n++)
			deck[n] = primes[j] | (j << 8) | suit | (1 << (16 + j));
}

void init_deck_another_way(int *deck)
{
    int suit = 0x8000;
    for (int i = 0; i < 4; i++, suit >>= 1)
        for (int j = 0; j < 13; j++)
			deck[j * 4 + i] = primes[j] | (j << 8) | suit | (1 << (16 + j));
}

short eval_5cards(int c1, int c2, int c3, int c4, int c5)
{
    int q;
    short s;
    q = (c1 | c2 | c3 | c4 | c5) >> 16;
    if (c1 & c2 & c3 & c4 & c5 & 0xF000) // check for flushes and straight-flushes
		return flushes[q];
    s = unique5[q]; // check for straights and high-card hands
    if (s) 
    	return s;
    q = (c1 & 0xFF) * (c2 & 0xFF) * (c3 & 0xFF) * (c4 & 0xFF) * (c5 & 0xFF);
    q = cactus_findit(q);
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
    int q, best = 9999, subhand[5];
	for (int i = 0; i < 21; i++)
	{
		for (int j = 0; j < 5; j++)
			subhand[j] = hand[perm7[i][j]];
		q = eval_5hand(subhand);
		if (q < best)
			best = q;
	}
	return best;
}

int n_cards = 0;

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
	if (id == 0)
		return 0; 

	if (id >= (*max_id))
	{           		
		if (id > (*max_id))
		{        
			ids[(*num_ids)++] = id;
			(*max_id) = id;
		}
		return (*num_ids) - 1;
	}

	// pseudo bsearch
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
			return holdtest;
	}
	memmove(&ids[high + 1], &ids[high], ((*num_ids) - high) * sizeof(ids[0]));  

	ids[high] = id;
	(*num_ids)++;        
	return high;
}

int do_eval(int64_t id_in)
{
	int n, handrank = 0, wcard, rank, suit, suititerator = 0, holdrank,
		mainsuit = 20, wcards[8], holdcards[8], numevalcards = 0;
	// const int primes[] = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41};
	
	memset(wcards, 0, sizeof(wcards));
	memset(holdcards, 0, sizeof(holdcards));

	if (id_in)
	{
		for (n = 0; n < 7; n++)
		{  
			holdcards[n] =  (int) ((id_in >> (8 * n)) & 0xff); 
			if (holdcards[n] == 0) 
				break;
			numevalcards++;
			if ((suit = holdcards[n] & 0xf))
				mainsuit = suit;
		}

		for (n = 0; n < numevalcards; n++)
		{  
			wcard = holdcards[n];

			// convert to cactus kevs way, ref: http://www.suffecool.net/poker/evaluator.html
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
			wcards[n] = primes[rank] | (rank << 8) | (1 << (suit + 11)) | (1 << (16 + rank));
		}

		switch (numevalcards)
		{  
			case 5:  
				holdrank = eval_5cards(wcards[0],wcards[1],wcards[2],wcards[3],wcards[4]);
				break;
			case 6:  
				holdrank = eval_5cards(wcards[0], wcards[1], wcards[2], wcards[3], wcards[4]);
				holdrank = MIN(holdrank, eval_5cards(wcards[0], wcards[1], wcards[2], wcards[3], wcards[5]));
				holdrank = MIN(holdrank, eval_5cards(wcards[0], wcards[1], wcards[2], wcards[4], wcards[5]));
				holdrank = MIN(holdrank, eval_5cards(wcards[0], wcards[1], wcards[3], wcards[4], wcards[5]));
				holdrank = MIN(holdrank, eval_5cards(wcards[0], wcards[2], wcards[3], wcards[4], wcards[5]));
				holdrank = MIN(holdrank, eval_5cards(wcards[1], wcards[2], wcards[3], wcards[4], wcards[5]));
				break;
			case 7: 
				holdrank = eval_7hand(wcards);  
				break;
			default:
				return -1;
		}

		handrank = cactus_to_ray(holdrank);
	}
	return handrank;
}

int raygen7(const char *filename, bool test=true)
{
	int card, count = 0, num_ids = 1, n, id_slot, max_handrank = 0, handTypeSum[10], holdid;
	int64_t ID, max_id = 0LLU;
	int *_HR = (int *) malloc(32487834 * sizeof(int));
	int64_t *ids = (int64_t *) malloc(612978 * sizeof(int64_t));

	memset(handTypeSum, 0, sizeof(handTypeSum));
	memset((long long *) ids, 0LLU, sizeof(ids));
	memset((int *) _HR, 0, sizeof(_HR));

	for (n = 0; ids[n] || n == 0; n++)
	{  
		for (card = 1; card < 53; card++)
		{	 
			ID = make_id(ids[n], card);
			if (n_cards < 7) 
				holdid = save_id(ID, ids, &max_id, &num_ids);
		}
		if ((n + 2) % 19 == 0)
			std::cout << "\rGenerating card IDs...  " << std::fixed << std::setw(6) <<
				(n + 1) << " / 612977";
	}
	std::cout << "\n";
	for (n = 0; ids[n] || n == 0; n++)
	{  
		for (card = 1; card < 53; card++) 
		{
			ID = make_id(ids[n], card);
			if (n_cards < 7)
				id_slot = save_id(ID, ids, &max_id, &num_ids) * 53 + 53;
			else
				id_slot = do_eval(ID);
			if (id_slot == -1)
			{
				std::cout << "    Error: problem with n_cards = " << n_cards << ".\n";
				free(_HR); free(ids); _HR = 0; ids = 0; return 1;
			}
			max_handrank = n * 53 + card + 53;
			_HR[max_handrank] = id_slot;
		}
		if (n_cards == 6 || n_cards == 7) 
		{  
			int value = do_eval(ids[n]);
			if (value == -1)
			{
				std::cout << "    Error: problem with n_cards = " << n_cards << ".\n";
				free(_HR); free(ids); _HR = 0; ids = 0; return 1;
			}
			_HR[n * 53 + 53] = value;
		}
		if ((n + 2) % 19 == 0)
			std::cout << "\rSetting hand ranks...  " << std::fixed << std::setw(6) <<
				(n + 1) << " / 612977";
	}
	std::cout << "\nThe highest hand rank: " << max_handrank << ".";

	int result = smart_save(_HR, 32487834, filename);
	free(_HR); free(ids); _HR = 0; ids = 0;
	if (result)
		return result;

	if (test)
	{
		std::cout << "\nRunning tests...";
		_HR = smart_load(filename);
		for (int c0 = 1; c0 < 53; c0++) {
			int u0 = _HR[53+c0];
			for (int c1 = c0+1; c1 < 53; c1++) {
				int u1 = _HR[u0+c1];
				for (int c2 = c1+1; c2 < 53; c2++) {
					int u2 = _HR[u1+c2];
					for (int c3 = c2+1; c3 < 53; c3++) {
						int u3 = _HR[u2+c3];
						for (int c4 = c3+1; c4 < 53; c4++) {
							int u4 = _HR[u3+c4];
							for (int c5 = c4+1; c5 < 53; c5++) {
								int u5 = _HR[u4+c5];
								for (int c6 = c5+1; c6 < 53; c6++) {
									handTypeSum[_HR[u5+c6] >> 12]++;
									count++;
								}
							}
						}
					}
				}
			}
		}
		for (int i = 0; i <= 9; i++)
			std::cout << std::endl << std::fixed << std::setw(16) << hand_rank_str(i) <<
				": " << handTypeSum[i];
		std::cout << "\nTotal hands = " << count << "\n";
		free(_HR); _HR = 0;
	}

	return 0;

	// FILE * fout = fopen(filename, "wb");
	// int result = 1;
	// if (fout)
	// {
	// 	result = 0;
 //       	fwrite(_HR, sizeof(int), 32487834, fout);
	// 	fclose(fout);
	// }
	// free(_HR); free(ids); _HR = 0; ids = 0;
	// return result;
}
