
#include "poker.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdint.h>
#include <CoreServices/CoreServices.h>
#include <mach/mach.h>
#include <mach/mach_time.h>
#include <unistd.h>
// #include <Kernel/kern/clock.h>

#include "arrays.h"
// #include "poker.h"

// Poker hand evaluator
//
// Kevin L. Suffecool
// suffecool@bigfoot.com
//

void    srand48();
double  drand48();

// perform a binary search on a pre-sorted array
//
int findit( int key )
{
    int low = 0, high = 4887, mid;

    while ( low <= high )
    {
        mid = (high+low) >> 1;      // divide by two
        if ( key < products[mid] )
            high = mid - 1;
        else if ( key > products[mid] )
            low = mid + 1;
        else
            return( mid );
    }
    fprintf( stderr, "ERROR:  no match found; key = %d\n", key );
    return( -1 );
}

//
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
//   p = prime number of rank (deuce=2,trey=3,four=5,five=7,...,ace=41)
//   r = rank of card (deuce=0,trey=1,four=2,five=3,...,ace=12)
//   cdhs = suit of card
//   b = bit turned on depending on rank of card
//
void init_deck( int *deck )
{
    int i, j, n = 0, suit = 0x8000;

    for ( i = 0; i < 4; i++, suit >>= 1 )
        for ( j = 0; j < 13; j++, n++ )
            deck[n] = primes[j] | (j << 8) | suit | (1 << (16+j));
}


//  This routine will search a deck for a specific card
//  (specified by rank/suit), and return the INDEX giving
//  the position of the found card.  If it is not found,
//  then it returns -1
//
int
find_card( int rank, int suit, int *deck )
{
    int i, c;

	for ( i = 0; i < 52; i++ )
	{
		c = deck[i];
		if ( (c & suit)  &&  (RANK(c) == rank) )
			return( i );
	}
	return( -1 );
}


//
//  This routine takes a deck and randomly mixes up
//  the order of the cards.
//
void shuffle_deck( int *deck )
{
    int i, n, temp[52];

    for ( i = 0; i < 52; i++ )
        temp[i] = deck[i];

    for ( i = 0; i < 52; i++ )
    {
        do {
            n = (int)(51.9999999 * drand48());
        } while ( temp[n] == 0 );
        deck[i] = temp[n];
        temp[n] = 0;
    }
}


void print_hand( int *hand, int n )
{
    int i, r;
    char suit;
    const char *rank = "23456789TJQKA";

    for ( i = 0; i < n; i++ )
    {
        r = (*hand >> 8) & 0xF;
        if ( *hand & 0x8000 )
            suit = 'c';
        else if ( *hand & 0x4000 )
            suit = 'd';
        else if ( *hand & 0x2000 )
            suit = 'h';
        else
            suit = 's';

        printf( "%c%c ", rank[r], suit );
        hand++;
    }
}


int
hand_rank( short val )
{
    if (val > 6185) return(HIGH_CARD);        // 1277 high card
    if (val > 3325) return(ONE_PAIR);         // 2860 one pair
    if (val > 2467) return(TWO_PAIR);         //  858 two pair
    if (val > 1609) return(THREE_OF_A_KIND);  //  858 three-kind
    if (val > 1599) return(STRAIGHT);         //   10 straights
    if (val > 322)  return(FLUSH);            // 1277 flushes
    if (val > 166)  return(FULL_HOUSE);       //  156 full house
    if (val > 10)   return(FOUR_OF_A_KIND);   //  156 four-kind
    return(STRAIGHT_FLUSH);                   //   10 straight-flushes
}


short
eval_5cards( int c1, int c2, int c3, int c4, int c5 )
{
    int q;
    short s;

    q = (c1|c2|c3|c4|c5) >> 16;

    /* check for Flushes and StraightFlushes
    */
    if ( c1 & c2 & c3 & c4 & c5 & 0xF000 )
	return( flushes[q] );

    /* check for Straights and HighCard hands
    */
    s = unique5[q];
    if ( s )  return ( s );

    /* let's do it the hard way
    */
    q = (c1&0xFF) * (c2&0xFF) * (c3&0xFF) * (c4&0xFF) * (c5&0xFF);
    q = findit( q );

    return( values[q] );
}


short
eval_5hand( int *hand )
{
    int c1, c2, c3, c4, c5;

    c1 = *hand++;
    c2 = *hand++;
    c3 = *hand++;
    c4 = *hand++;
    c5 = *hand;

    return( eval_5cards(c1,c2,c3,c4,c5) );
}


// This is a non-optimized method of determining the
// best five-card hand possible out of seven cards.
// I am working on a faster algorithm.
//
short
eval_7hand( int *hand )
{
    int i, j, q, best = 9999, subhand[5];

	for ( i = 0; i < 21; i++ )
	{
		for ( j = 0; j < 5; j++ )
			subhand[j] = hand[ perm7[i][j] ];
		q = eval_5hand( subhand );
		if ( q < best )
			best = q;
	}
	return( best );
}



const char HandRanks[][16] = {"BAD!!","High Card","Pair","Two Pair","Three of a Kind","Straight","Flush","Full House","Four of a Kind","Straight Flush"};


// Aldanor: some hacks
#define __int64 int64_t
#define __min(a,b)  (((a) < (b)) ? (a) : (b))

__int64 IDs[612978];
int HR[32487834];   

int numIDs = 1;
int numcards = 0;
int maxHR = 0;
__int64 maxID = 0;

//Raw mach_absolute_times going in, difference in seconds out
double subtractTimes( uint64_t endTime, uint64_t startTime )
{
    uint64_t difference = endTime - startTime;
    static double conversion = 0.0;
    
    if( conversion == 0.0 )
    {
        mach_timebase_info_data_t info;
        kern_return_t err = mach_timebase_info( &info );
        
	//Convert the timebase into seconds
        if( err == 0  )
		conversion = 1e-9 * (double) info.numer / (double) info.denom;
    }
    
    return conversion * (double) difference;
}

__int64 MakeID(__int64 IDin, int newcard)  // adding a new card to this ID
{
	__int64 ID = 0;
	int suitcount[4 + 1];
	int rankcount[13 + 1];
	int workcards[8];  // intentially keeping one as a 0 end
	int cardnum;
	int getout = 0;
	
	memset(workcards, 0, sizeof(workcards));
	memset(rankcount, 0, sizeof(rankcount));
	memset(suitcount, 0, sizeof(suitcount));
	
	for (cardnum = 0; cardnum < 6; cardnum++) {  // can't have more than 6 cards!
		workcards[cardnum + 1] =  (int) ((IDin >> (8 * cardnum)) & 0xff);  // leave the 0 hole for new card
	}

	// my cards are 2c = 1, 2d = 2  ... As = 52
	newcard--;  // make 0 based!

	workcards[0] = (((newcard >> 2) + 1) << 4) + (newcard & 3) + 1;  // add next card formats card to rrrr00ss

	for (numcards = 0; workcards[numcards]; numcards++) {
		suitcount[workcards[numcards] & 0xf]++;           // need to see if suit is significant
		rankcount[(workcards[numcards] >> 4) & 0xf]++;	  // and rank to be sure we don't have 4!
		if (numcards) {
			if (workcards[0] == workcards[numcards]) {	  // can't have the same card twice
				getout = 1;								  // if so need to get out after counting numcards
			}
		}
	}

	if (getout) {
		return 0;     // duplicated another card (ignore this one)    
	}

	
	int needsuited = numcards - 2;	   // for suit to be significant - need to have n-2 of same suit
	     
	if (numcards > 4) {  
		for (int rank = 1; rank < 14; rank++) {
			if (rankcount[rank] > 4) {  // if I have more than 4 of a rank then I shouldn't do this one!!
				return 0;   // can't have more than 4 of a rank so return an ID that can't be!
			}
		}
	}
	
	// However in the ID process I prefered that
	// 2s = 0x21, 3s = 0x31,.... Kc = 0xD4, Ac = 0xE4
	// This allows me to sort in Rank then Suit order
	
	// if we don't have at least 2 cards of the same suit for 4, we make this card suit 0.
	
	if (needsuited > 1) { 
		for (cardnum = 0; cardnum < numcards; cardnum++) {  // for each card
			if (suitcount[workcards[cardnum] & 0xf] < needsuited) {	// check suitcount to the number I need to have suits significant
				workcards[cardnum] &= 0xf0;   // if not enough - 0 out the suit - now this suit would be a 0 vs 1-4
			}
		}
	}

	// Sort Using XOR.  Network for N=7, using Bose-Nelson Algorithm: Thanks to the thread!
#define SWAP(I,J) {if (workcards[I] < workcards[J]) {workcards[I]^=workcards[J]; workcards[J]^=workcards[I]; workcards[I]^=workcards[J];}}		

	SWAP(0, 4);		
	SWAP(1, 5);		
	SWAP(2, 6);		
	SWAP(0, 2);		
	SWAP(1, 3);		
	SWAP(4, 6);		
	SWAP(2, 4);		
	SWAP(3, 5);		
	SWAP(0, 1);		
	SWAP(2, 3);		
	SWAP(4, 5);		
	SWAP(1, 4);		
	SWAP(3, 6);		
	SWAP(1, 2);		
	SWAP(3, 4);		
	SWAP(5, 6);	

	// long winded way to put the pieces into a __int64 
	// cards in bytes --66554433221100	 
	// the resulting ID is a 64 bit value with each card represented by 8 bits.
	ID =  (__int64) workcards[0] +
		 ((__int64) workcards[1] << 8) +
		 ((__int64) workcards[2] << 16) + 
		 ((__int64) workcards[3] << 24) +
		 ((__int64) workcards[4] << 32) +
		 ((__int64) workcards[5] << 40) +
		 ((__int64) workcards[6] << 48);    
	
	return ID;
}

int SaveID(__int64 ID) 
{
	if (ID == 0) return 0;   // don't use up a record for a 0!

	if (ID >= maxID) {           // take care of the most likely first goes on the end...
		if (ID > maxID) {        // greater than create new else it was the last one!
			IDs[numIDs++] = ID;  // add the new ID
			maxID = ID;
		}
		return numIDs - 1;
	}

	// find the slot I will find it (by a pseudo bsearch algorithm)
	int low = 0;
	int high = numIDs - 1;
	__int64 testval;
	int holdtest;

	while (high - low > 1) {
		holdtest = (high + low + 1) / 2;
		testval = IDs[holdtest] - ID;
		if (testval > 0) high = holdtest;
		else if (testval < 0) low = holdtest;
		else return holdtest;   // got it!!
	}
	// I guess it couldn't be found so must be added to the current location (high)
	// make space...  // don't expect this much!
	memmove(&IDs[high + 1], &IDs[high], (numIDs - high) * sizeof(IDs[0]));  

	IDs[high] = ID;   // do the insert into the hole created
	numIDs++;        
	return high;
}

int DoEval(__int64 IDin)
{
	// I guess I have some explaining to do here...  I used the Cactus Kevs Eval ref http://www.suffecool.net/poker/evaluator.html
	// I Love the pokersource for speed, but I needed to do some tweaking to get it my way
	// and Cactus Kevs stuff was easy to tweak ;-)  
	int handrank = 0;
	int cardnum;
	int workcard;
	int rank;
	int suit;
	int mainsuit = 20;      // just something that will never hit...  need to eliminate the main suit from the iterator
	int suititerator = 0;
	int holdrank;
	int workcards[8];  // intentially keeping one as a 0 end
	int holdcards[8];
	int numevalcards = 0;

	// See Cactus Kevs page for explainations for this type of stuff...
	const int primes[] = { 2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41 };
	
	memset(workcards, 0, sizeof(workcards));
	memset(holdcards, 0, sizeof(holdcards));

	if (IDin) {	 // if I have a good ID then do it...
		for (cardnum = 0; cardnum < 7; cardnum++) {  // convert all 7 cards (0s are ok)
			holdcards[cardnum] =  (int) ((IDin >> (8 * cardnum)) & 0xff); 
			if (holdcards[cardnum] == 0) break;	// once I hit a 0 I know I am done
			numevalcards++;						// if not 0 then count the card
			if (suit = holdcards[cardnum] & 0xf) {	// find out what suit (if any) was significant
				mainsuit = suit;					// and remember it
			}
		}


		for (cardnum = 0; cardnum < numevalcards; cardnum++) {  // just have numcards...
			workcard = holdcards[cardnum];

			// convert to cactus kevs way!!  ref http://www.suffecool.net/poker/evaluator.html
			//   +--------+--------+--------+--------+
			//   |xxxbbbbb|bbbbbbbb|cdhsrrrr|xxpppppp|
			//   +--------+--------+--------+--------+
			//   p = prime number of rank (deuce=2,trey=3,four=5,five=7,...,ace=41)
			//   r = rank of card (deuce=0,trey=1,four=2,five=3,...,ace=12)
			//   cdhs = suit of card
			//   b = bit turned on depending on rank of card

			rank = (workcard >> 4) - 1;	 // my rank is top 4 bits 1-13 so convert
			suit = workcard & 0xf;  // my suit is bottom 4 bits 1-4, order is different, but who cares?  
			if (suit == 0) {		// if suit wasn't significant though...
				suit = suititerator++;   // Cactus Kev needs a suit!
				if (suititerator == 5)	 // loop through available suits
					suititerator = 1;
				if (suit == mainsuit) {   // if it was the sigificant suit...  Don't want extras!!
					suit = suititerator++;    // skip it
					if (suititerator == 5)	  // roll 1-4
						suititerator = 1;
				}
			}
			// now make Cactus Keys Card
			workcards[cardnum] = primes[rank] | (rank << 8) | (1 << (suit + 11)) | (1 << (16 + rank));
		}

		switch (numevalcards) {  // run Cactus Keys routines
			case 5 :  holdrank = eval_5cards(workcards[0],workcards[1],workcards[2],workcards[3],workcards[4]);
				      break;
					  // if 6 cards I would like to find HandRank for them 
					  // Cactus Key is 1 = highest - 7362 lowest I need to get the min for the permutations
			case 6 :  holdrank = eval_5cards(workcards[0],workcards[1],workcards[2],workcards[3],workcards[4]);
				      holdrank = __min(holdrank, eval_5cards(workcards[0],workcards[1],workcards[2],workcards[3],workcards[5]));
				      holdrank = __min(holdrank, eval_5cards(workcards[0],workcards[1],workcards[2],workcards[4],workcards[5]));
				      holdrank = __min(holdrank, eval_5cards(workcards[0],workcards[1],workcards[3],workcards[4],workcards[5]));
				      holdrank = __min(holdrank, eval_5cards(workcards[0],workcards[2],workcards[3],workcards[4],workcards[5]));
				      holdrank = __min(holdrank, eval_5cards(workcards[1],workcards[2],workcards[3],workcards[4],workcards[5]));
					  break;
			case 7 : holdrank = eval_7hand(workcards);  
				     break;
			default : // problem!!  shouldn't hit this... 
				      printf("    Problem with numcards = %d!!\n", numcards);
					  break;
		}

// I would like to change the format of Catus Kev's ret value to:
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
	return handrank;  // now a handrank that I like
}


// int _tmain(int argc, _TCHAR* argv[])
int main(int argc, char* argv[])
{
	int count = 0;
	int card;

	__int64 ID;
	int IDslot;

	clock_t timer = clock();   // remember when I started

	int handTypeSum[10];
	
	memset(handTypeSum, 0, sizeof(handTypeSum));  // init
	memset(IDs, 0, sizeof(IDs));
	memset(HR, 0, sizeof(HR));


  // step through the ID array - always shifting the current ID and adding 52 cards to the end of the array.
	// when I am at 7 cards put the Hand Rank in!!  
	// stepping through the ID array is perfect!!
	
    int IDnum;
	int holdid;

	printf("\nGetting Card IDs!\n");
	// as this loops through and find new combinations it adds them to the end
	// I need this list to be stable when I set the handranks (next set)  (I do the insertion sort on new IDs these)
	// so I had to get the IDs first and then set the handranks
	for (IDnum = 0; IDs[IDnum] || IDnum == 0; IDnum++) {  // start at 1 so I have a zero catching entry (just in case)
		for (card = 1; card < 53; card++) {	 // the ids above contain cards upto the current card.  Now add a new card 
			ID = MakeID(IDs[IDnum], card);   // get the new ID for it
			if (numcards < 7) holdid = SaveID(ID);   // and save it in the list if I am not on the 7th card
		}
		printf("\rID - %d", IDnum);	  // just to show the progress -- this will count up to  612976
	}
	
	printf("\nSetting HandRanks!\n");
	// this is as above, but will not be adding anything to the ID list, so it is stable
	for (IDnum = 0; IDs[IDnum] || IDnum == 0; IDnum++) {  // start at 1 so I have a zero catching entry (just in case)
		for (card = 1; card < 53; card++) {
			ID = MakeID(IDs[IDnum], card);
			if (numcards < 7)  IDslot = SaveID(ID) * 53 + 53;  // when in the index mode (< 7 cards) get the id to save
			else IDslot = DoEval(ID);   // if I am at the 7th card, get the HandRank to save
			maxHR = IDnum * 53 + card + 53;	// find where to put it 
			HR[maxHR] = IDslot;				// and save the pointer to the next card or the handrank
		}

		if (numcards == 6 || numcards == 7) {  
			// an extra, If you want to know what the handrank when there is 5 or 6 cards
			// you can just do HR[u3] or HR[u4] from below code for Handrank of the 5 or 6 card hand
			HR[IDnum * 53 + 53] = DoEval(IDs[IDnum]);  // this puts the above handrank into the array  
		}
		printf("\rID - %d", IDnum);	  // just to show the progress -- this will count up to  612976 same as above!
	}

	printf("\nNumber IDs = %d\nmaxHR = %d\n", numIDs, maxHR);  // for warm fuzzys

	timer = clock() - timer;  // end the timer
	
	printf("Training seconds = %.2f\n", (float)timer/CLOCKS_PER_SEC);  // display training time

	// LARGE_INTEGER timings, endtimings;	// for high precision timing

	timer = clock();   // now get current time for Testing!  

	// another algorithm right off the thread

	int c0, c1, c2, c3, c4, c5, c6;
	int u0, u1, u2, u3, u4, u5;

	// QueryPerformanceCounter(&timings);				    // start High Precision clock
	uint64_t start, end, elapsed;
	start = mach_absolute_time();

	for (c0 = 1; c0 < 53; c0++) {
		u0 = HR[53+c0];
		for (c1 = c0+1; c1 < 53; c1++) {
			u1 = HR[u0+c1];
			for (c2 = c1+1; c2 < 53; c2++) {
				u2 = HR[u1+c2];
				for (c3 = c2+1; c3 < 53; c3++) {
					u3 = HR[u2+c3];
					for (c4 = c3+1; c4 < 53; c4++) {
						u4 = HR[u3+c4];
						for (c5 = c4+1; c5 < 53; c5++) {
							u5 = HR[u4+c5];
							for (c6 = c5+1; c6 < 53; c6++) {
								handTypeSum[HR[u5+c6] >> 12]++;
								count++;
							}
						}
					}
				}
			}
		}
	}
	
	end = mach_absolute_time();

	// QueryPerformanceCounter(&endtimings);	  // end the high precision clock
 
	timer = clock() - timer;  // get the time in this

	for (int i = 0; i <= 9; i++)  // display the results
		printf("\n%16s = %d", HandRanks[i], handTypeSum[i]);
	
	printf("\nTotal Hands = %d\n", count);
	
	// __int64 clocksused = (__int64)endtimings.QuadPart - (__int64) timings.QuadPart;  // calc clocks used from the High Precision clock

	// and display the clock results
	// printf("\nValidation seconds = %.4lf\nTotal HighPrecision Clocks = %I64d\nHighPrecision clocks per lookup = %lf\n", (double)timer/CLOCKS_PER_SEC, clocksused, (double) clocksused /  133784560.0) ;

	printf("\nElapsed: %1.10f us.\n", subtractTimes(end, start) * 1000000.);

	// output the array now that I have it!!
	FILE * fout = fopen("HandRanks.dat", "wb");
	if (!fout) {
		printf("Problem creating the Output File!\n");
		return 1;
	}
	fwrite(HR, sizeof(HR), 1, fout);  // big write, but quick
	
	fclose(fout);

	return 0;
}

///////////////////////////////// end code!!