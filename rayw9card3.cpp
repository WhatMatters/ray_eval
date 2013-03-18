#include <iostream>
#include <fstream>
#include <algorithm>
#include <vector>
#include <iomanip>
#include <locale>
#include <string>
#include <tr1/unordered_map>
#include <exception>

#include "arrays.h"
#include "load_file.h"

#define MAX(x, y) 	((x) > (y) ? (x) : (y))
#define MIN(x, y) 	((x) < (y) ? (x) : (y))

const int ANY_CARD = 1;	// placeholder for flush ranks eval
const int SKIP_BOARD = 53; // placeholder for 7-8 card hands

const int pocket_perms[6][2] = {
	{0, 1}, {0, 2}, {0, 3}, {1, 2}, {1, 3}, {2, 3}
};
const int board_perms[10][3] = {
	{0, 1, 2}, 															// 3-5 cards
	{0, 1, 3}, {0, 2, 3}, {1, 2, 3}, 									// 4-5 cards
	{0, 1, 4}, {0, 2, 4}, {0, 3, 4}, {1, 2, 4}, {1, 3, 4}, {2, 3, 4}	// 5 cards
}; 	
const int n_pocket_perms = 6;

void skip_board(int *board, int &n_board)
{
	int temp[5] = {0, 0, 0, 0, 0};
	n_board = 0;
	for (int i = 0; i < 5; i++)
		if (board[i] != SKIP_BOARD)
			temp[n_board++] = board[i];
	memcpy(board, temp, 5 * sizeof(int));
}

int cactus_to_ray(int holdrank)
{
	int handrank = 7463 - holdrank;  // now the worst hand = 1
		
	if      (handrank < 1278) handrank = handrank -    0 + 4096 * 1;  // 1277 high card
	else if (handrank < 4138) handrank = handrank - 1277 + 4096 * 2;  // 2860 one pair
	else if (handrank < 4996) handrank = handrank - 4137 + 4096 * 3;  //  858 two pair
	else if (handrank < 5854) handrank = handrank - 4995 + 4096 * 4;  //  858 three-kind
	else if (handrank < 5864) handrank = handrank - 5853 + 4096 * 5;  //   10 straights
	else if (handrank < 7141) handrank = handrank - 5863 + 4096 * 6;  // 1277 flushes
	else if (handrank < 7297) handrank = handrank - 7140 + 4096 * 7;  //  156 full house
	else if (handrank < 7453) handrank = handrank - 7296 + 4096 * 8;  //  156 four-kind
	else                      handrank = handrank - 7452 + 4096 * 9;  //   10 straight-flushes

	return handrank;	
}

int count_cards(int64_t id)
{
	return (((id) & 0x7F) != 0) + 
		(((id >> 7) & 0x7F) != 0) + 
		(((id >> 14) & 0x7F) != 0) + 
		(((id >> 21) & 0x7F) != 0) + 
		(((id >> 28) & 0x7F) != 0) + 
		(((id >> 35) & 0x7F) != 0) + 
		(((id >> 42) & 0x7F) != 0) + 
		(((id >> 49) & 0x7F) != 0) + 
		(((id >> 56) & 0x7F) != 0);
}

void add_card(int new_card, int *pocket, int *board, int &n_pocket, int &n_board)
{
	if (n_board < 5)
		board[n_board++] = new_card;
	else
		pocket[n_pocket++] = new_card;
}

void unpack64(int64_t id, int *pocket, int *board, int &n_pocket, int &n_board)
{
	memset(pocket, 0, 4 * sizeof(int));
	memset(board, 0, 5 * sizeof(int));
	n_pocket = 0;
	n_board = 0;
	int card;
	for (int i = 0; i < 9; i++)
	{
		if ((card = (int) ((id >> (7 * i)) & 0x7F)) != 0)
		{
			if (i < 5)
				board[n_board++] = card;
			else
				pocket[n_pocket++] = card;
		}
	}
}

int64_t pack64(int *pocket, int *board)
{
	std::sort(pocket, pocket + 4);
	std::sort(board, board + 5);
	std::reverse(pocket, pocket + 4);
	std::reverse(board, board + 5);

	return ((int64_t) board[0] +
		 ((int64_t) board[1] << 7) +
		 ((int64_t) board[2] << 14) + 
		 ((int64_t) board[3] << 21) +
		 ((int64_t) board[4] << 28) +
		 ((int64_t) pocket[0] << 35) +
		 ((int64_t) pocket[1] << 42) +
		 ((int64_t) pocket[2] << 49) +
		 ((int64_t) pocket[3] << 56));    
}

void print_id(int64_t id, bool indent=false)
{
	int board[5], pocket[4], n_pocket, n_board;
	unpack64(id, pocket, board, n_pocket, n_board);
	std::cout << (indent ? "\t" : "") << "id:       " << id << "\n";
	std::cout << (indent ? "\t" : "") << "n_board:  " << n_board << "\n";
	std::cout << (indent ? "\t" : "") << "board:    [";
	for (int i = 0; i < n_board; i++)
		std::cout << board[i] << (((i + 1) == n_board) ? "" : ", ");
	std::cout << "]\n";
	std::cout << (indent ? "\t" : "") << "n_pocket:  " << n_pocket << "\n";
	std::cout << (indent ? "\t" : "") << "pocket:   [";
	for (int i = 0; i < n_pocket; i++)
		std::cout << pocket[i] << (((i + 1) == n_pocket) ? "" : ", ");
	std::cout << "]\n";
}

int64_t add_card_to_id_flush_suits(int64_t id, int new_card)
{
	int pocket[4], board[5], n_pocket, n_board;
	new_card = new_card == 0 ? SKIP_BOARD : ((new_card - 1) & 3) + 1;
	unpack64(id, pocket, board, n_pocket, n_board);
	add_card(new_card, pocket, board, n_pocket, n_board);
	return pack64(pocket, board);
}

int eval_flush_suits(int64_t id)
{
	int pocket[4], board[5], n_pocket, n_board,
		nsp[5] = {0, 0, 0, 0, 0}, nsb[5] = {0, 0, 0, 0, 0};
	unpack64(id, pocket, board, n_pocket, n_board);
	for (int i = 0; i < n_pocket; i++)
		nsp[pocket[i]] = MIN(nsp[pocket[i]] + 1, 2);
	for (int i = 0; i < n_board; i++)
		if (board[i] != SKIP_BOARD)
			nsb[board[i]] = MIN(nsb[board[i]] + 1, 3);
	for (int suit = 1; suit <= 4; suit++)
		if ((nsp[suit] + nsb[suit]) >= 5)
			return suit;
	return -1;
}

int64_t add_card_to_id_flush_ranks(int64_t id, int new_card, int flush_suit)
{
	int pocket[4], board[5], n_pocket, n_board, nsp = 0, nsb = 0, i;
	bool debug = false;

	// the rank is 2-14 if suited or 1 otherwise (for sorting purposes)
	if (new_card == 0)
		new_card = SKIP_BOARD;
	else
		new_card = ((((new_card - 1) & 3) + 1) == flush_suit) ? 
			(2 + ((new_card - 1) >> 2) & 0xF) : ANY_CARD;

	if (debug) std::cout << "old deck:\n";
	if (debug) print_id(id, true);
	if (debug) std::cout << "new_card: " << new_card << "\n";

	unpack64(id, pocket, board, n_pocket, n_board);

	for (i = 0; i < n_pocket; i++)
		if (pocket[i] != ANY_CARD && pocket[i] != SKIP_BOARD && pocket[i] == new_card)
		{
			if (debug) std::cout << "halting, duplicate pocket: " << pocket[i] << "\n";
			return 0;
		}
	for (i = 0; i < n_board; i++)
		if (board[i] != ANY_CARD && board[i] != SKIP_BOARD && board[i] == new_card)
		{
			if (debug) std::cout << "halting, duplicate board: " << board[i] << "\n";
			return 0;
		}

	if (debug) std::cout << "no duplicates, adding card...\n";

	add_card(new_card, pocket, board, n_pocket, n_board);

	for (i = 0; i < n_pocket; i++)
		if (pocket[i] != ANY_CARD && pocket[i] != 0) 
			nsp++;
	for (i = 0; i < n_board; i++)
		if (board[i] != ANY_CARD && board[i] != SKIP_BOARD && board[i] != 0)
			nsb++;

	if (debug) std::cout << "nsp = " << nsp << ", nsb = " << nsb << "\n";

	if (n_board == 4 && nsb <= 1)
		return 0;
	if (n_board == 5 && nsb <= 2)
		return 0;
	if (n_board == 5 && n_pocket == 3 && nsp == 0)
		return 0;
	if (n_board == 5 && n_pocket == 4 && nsp <= 1)
		return 0;

	if (debug) std::cout << "packing...\n";

	if (debug) std::cout << "new deck:\n";
	if (debug) print_id(pack64(pocket, board), true);

	return pack64(pocket, board);
}

int64_t add_card_to_id_flush_ranks_1(int64_t id, int new_card)
{
	return add_card_to_id_flush_ranks(id, new_card, 1);	
}

int64_t add_card_to_id_flush_ranks_2(int64_t id, int new_card)
{
	return add_card_to_id_flush_ranks(id, new_card, 2);	
}

int64_t add_card_to_id_flush_ranks_3(int64_t id, int new_card)
{
	return add_card_to_id_flush_ranks(id, new_card, 3);	
}

int64_t add_card_to_id_flush_ranks_4(int64_t id, int new_card)
{
	return add_card_to_id_flush_ranks(id, new_card, 4);	
}

int eval_flush_ranks(int64_t id)
{
	int pocket[4], board[5], n_pocket, n_board;
	unpack64(id, pocket, board, n_pocket, n_board);
	skip_board(board, n_board);
	if (!pocket[0] || !pocket[1] || !board[0] || !board[1] || !board[2])
	{	
		std::cout << "\neval_flush_ranks(): " << id << ": zero encountered.\n";
		return -1;
	}
	if (pocket[0] == ANY_CARD || pocket[1] == ANY_CARD || board[0] == ANY_CARD ||
		board[1] == ANY_CARD || board[2] == ANY_CARD)
		return -1;

	// sadly, we have to account for straight flushes...
	int n = n_pocket + n_board;
	const int pocket_perms[2][6] = {{0, 0, 0, 1, 1, 2}, {1, 2, 3, 2, 3, 3}};
	const int n_pocket_perms = 6;
	const int board_perms[10][3] = {
		{0, 1, 2}, // 3, 4, 5
		{0, 1, 3}, {0, 2, 3}, {1, 2, 3}, // 4, 5
		{0, 1, 4}, {0, 2, 4}, {0, 3, 4}, {1, 2, 4}, {1, 3, 4}, {2, 3, 4}}; // 5
	int n_board_perms = n == 9 ? 10 : n == 8 ? 4 : n == 7 ? 1 : -1;
	int np, nb, best = 8191, q = 0;
	for (np = 0; np < n_pocket_perms; np++)
	{
		for (nb = 0; nb < n_board_perms; nb++)
		{
			int rank1 = pocket[pocket_perms[0][np]] - 2,
				rank2 = pocket[pocket_perms[1][np]] - 2,
				rank3 = board[board_perms[nb][0]] - 2,
				rank4 = board[board_perms[nb][1]] - 2,
				rank5 = board[board_perms[nb][2]] - 2;
			if (rank1 < 0 || rank2 < 0 || rank3 < 0 || rank4 < 0 || rank5 < 0 ||
				rank1 > 12 || rank2 > 12 || rank3 > 12 || rank4 > 12 || rank5 > 12)
				continue;
			q = flushes[(1 << rank1) | (1 << rank2) | (1 << rank3) | (1 << rank4) | (1 << rank5)];
			if (q < best)
				best = q;
		}
	}
	return cactus_to_ray(best);
}

int64_t add_card_to_id_no_flush(int64_t id, int new_card)
{
	int pocket[4], board[5], n_pocket, n_board, i,
		n_rank[14] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	if (new_card == 0)
		new_card = SKIP_BOARD;
	else
		new_card = 1 + ((new_card - 1) >> 2) & 0xF;
	unpack64(id, pocket, board, n_pocket, n_board);
	for (i = 0; i < n_pocket; i++)
		n_rank[pocket[i]]++;
	for (i = 0; i < n_board; i++)
		if (board[i] != SKIP_BOARD)
			n_rank[board[i]]++;
	add_card(new_card, pocket, board, n_pocket, n_board);
	if (new_card != SKIP_BOARD)
		n_rank[new_card]++;
	for (i = 1; i <= 13; i++)
		if (n_rank[i] > 4)
			return 0;
	return pack64(pocket, board);
}

int card_to_cactus(int rank, int suit)
{
	static bool buffered = false;
	static int cards[14][5];
	const int primes[13] = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41};
	if (!buffered)
	{
		for (int r = 0; r < 14; r++)
			cards[r][0] = 0;
		for (int s = 0; s < 5; s++)
			cards[0][s] = 0;
		for (int r = 1; r < 14; r++)
			for (int s = 1; s < 5; s++)
				cards[r][s] = primes[(r - 1)] | ((r - 1) << 8) | 
					(1 << (s + 11)) | (1 << (16 + (r - 1)));
		buffered = true;
	}
	return cards[rank][suit];
}

int cactus_findit(int key)
{
    int low = 0, high = 4887, mid;
    while (low <= high)
    {
        mid = (high + low) >> 1;
        if (key < products[mid])
            high = mid - 1;
        else if (key > products[mid])
            low = mid + 1;
        else
            return mid;
    }
    std::cout << "\ncactus_findit(): no match found; key = " << key << "\n";
    return -1;
}

int eval_cactus_no_flush(int c1, int c2, int c3, int c4, int c5)
{
	int s = unique5[(c1 | c2 | c3 | c4 | c5) >> 16];
	if (s)
		return s;
	else
		return values[cactus_findit((c1 & 0xFF) * (c2 & 0xFF) * 
			(c3 & 0xFF) * (c4 & 0xFF) * (c5 & 0xFF))];
}

int eval_no_flush(int64_t id)
{

	int pocket[4], board[5], n_pocket, n_board, n, n_board_perms;
	unpack64(id, pocket, board, n_pocket, n_board);
	skip_board(board, n_board);
	n = n_pocket + n_board;
	n_board_perms = n == 9 ? 10 : n == 8 ? 4 : n == 7 ? 1 : -1;
	if (n_pocket < 4 || n_board < 3)
	{	
		std::cout << "\neval_no_flush() encountered invalid # of cards, shouldn't happen...\n";
		return 0;
	}

	// convert to cactus and add random suits
	int suit = 0;
	for (int i = 0; i < n_pocket; i++)
		pocket[i] = card_to_cactus(pocket[i], ((suit++) % 4) + 1);
	for (int i = 0; i < n_board; i++)
		board[i] = card_to_cactus(board[i], ((suit++) % 4) + 1);

	int np, nb, best = 8191, q = 0;
	for (np = 0; np < n_pocket_perms; np++)
	{
		for (nb = 0; nb < n_board_perms; nb++)
		{
			q = eval_cactus_no_flush(pocket[pocket_perms[np][0]],
				pocket[pocket_perms[np][1]],
				board[board_perms[nb][0]],
				board[board_perms[nb][1]],
				board[board_perms[nb][2]]);
			if (q < best)
				best = q;
		}
	}
	return cactus_to_ray(best);
}

/*
	Allow to "skip" a board card only when there are no cards on the
	table or just one skipped. A board card can be skipped by passing
	the offset of 0, however internally it will be stored as 53. 
	Skipping is only allowed for the first and second cards (essentially, 
	we just start off from a different offset).
*/

void generate_ids(size_t size, std::vector<int64_t> &id_list,
	int64_t (*add_card_to_id) (int64_t, int))

{
	std::vector<int64_t> id_queue_1, id_queue_2;
	int64_t new_id = 0;
	id_list.reserve(size);
	id_list.clear();
	id_list.push_back(0LL);
	id_queue_1.reserve(size);
	id_queue_2.reserve(size);
	id_queue_1.push_back(0LL);
	for (int n_cards = 1; n_cards <= 8; n_cards++)
	{
		std::cout << "\nGenerating " << n_cards << "-card IDs:\n";

		int n1 = id_queue_1.size(); // LOL, if we don't do this, expect nasty bugs
		for (int i = 0; i < n1; i++)
		{
			std::cout << "\r\t" << "Processing ID " << i + 1 << " / " <<
				n1 << "...";

			int64_t id = id_queue_1[i];
			int min_card = (n_cards <= 2) ? 0 : 1; // board skipping
			for (int new_card = min_card; new_card <= 52; new_card++)
				if ((new_id = (*add_card_to_id)(id, new_card)) != 0)
					id_queue_2.push_back(new_id);
		}
		std::cout << "\n\t" << id_queue_1.size() << ", " << id_queue_2.size() << "\n";
		size_t size = id_queue_2.size();
		std::cout << "\n\tGenerated " << id_queue_2.size() << " IDs." <<
			"\n\tSorting and dropping duplicates...";
		std::sort(id_queue_2.begin(), id_queue_2.end());
		id_queue_2.erase(std::unique(id_queue_2.begin(), id_queue_2.end()), 
			id_queue_2.end());
		std::cout << " dropped " << (size - id_queue_2.size()) << " IDS.";
		std::cout << "\n\tInserting IDS into the final list...";
		id_list.insert(id_list.end(), id_queue_2.begin(), id_queue_2.end());
		std::cout << " total: " << id_list.size() << " IDS.";
		std::cout << "\n\tResetting the queue...";
		id_queue_1.swap(id_queue_2);
		id_queue_2.clear();
	}
	std::cout << "\n\tFinished: generated " << id_list.size() << " IDs, sorting....";
	std::sort(id_list.begin(), id_list.end());
	std::vector<int64_t>().swap(id_queue_1);
	std::vector<int64_t>().swap(id_queue_2);
}

/*

GLOBAL

0-53
	0: flush ranks base offset (add suit * 53 to get true offset)
	1: 

FLUSH_RANKS
	0-53: 			reserved
	53*1 + 1-52:	suit 1 starting point
	53*2 + 1-52:	suit 2 starting point
	53*3 + 1-52:	suit 3 starting point
	53*4 + 1-52:	suit 4 starting point
	>= 53*5: 		normal layers

FLUSH_RANKS SHIFTING

	Assume: there are only 2 ranks, 1-2 and 4 suits, 1-4
	Denote:  -- , [1], [2] ~ any card, rank 1, rank 2
	        [b], [d] ~ base offset, dummy slot

		      [b]  1(1) 1(2) 1(3) 1(4) 2(1) 2(2) 2(3) 2(4) [d]  [d]  [d]

	suit 1:        [1]   --  --   --   [2]  --   --   --   --   --   -- 
	suit 2:         --  [1]  --   --   --   [2]  --   --   --   --   -- 
	suit 4:         --   --  [1]  --   --   --   [2]  --   --   --   -- 
	suit 3:         --   --  --   [1]  --   --   --   [2]  --   --   -- 
*/

// void process_ids(std::vector<int64_t> ids, int offset, int offset_value,
// 	int *hand_ranks, int64_t (*add_card_to_id)(int64_t, int),
// 	int (*eval_id)(int64_t), std::tr1::unordered_map<int, int> map=
// 	std::tr1::unordered_map<int, int>())
void process_ids(std::vector<int64_t> ids, int offset, int offset_value,
	std::vector<int> &hand_ranks, int64_t (*add_card_to_id)(int64_t, int),
	int (*eval_id)(int64_t), std::tr1::unordered_map<int, int> map=
	std::tr1::unordered_map<int, int>())
{
	// offset + 0: special value
	// offset + 1-52: loop back to offset + 0
	// offset + 53: starting point
	// offset + 54+: normal operation

	int n = ids.size(), i, id_index, num_cards, new_card;
	int64_t id, new_id;
	std::tr1::unordered_map<int64_t, int> hash_table;

	for (i = 0; i < n; i++)
	{
		std::cout << "\r\tCreating unordered map... " << (i + 1) << " / " << n;
		hash_table.insert(std::tr1::unordered_map<int64_t, int>::value_type(ids[i], i));
	}
	std::cout << "\n";

	hand_ranks[offset] = offset_value;
	for (i = 1; i <= 52; i++)
		hand_ranks[offset + i] = offset;
	for (i = 0; i < n; i++)
	{
		id = ids[i];
		std::cout << "\r\tProcessing ID " << i + 1 << " out of " << n << " (" << id << ")..." ;
		id_index = offset + 53 + i * 53;
		num_cards = count_cards(id);
		hand_ranks[id_index] = offset; // safety backup

		int min_card = (num_cards <= 1) ? 0 : 1; // board skipping
		for (new_card = min_card; new_card <= 52; new_card++)
		{			
			new_id = (*add_card_to_id)(id, new_card);
			if (new_id && ((num_cards + 1) == 9))
			{
				int value = (*eval_id)(new_id);
				hand_ranks[id_index + new_card] = map.count(value) ? map[value] : value;
			}
			else if (new_id)
				hand_ranks[id_index + new_card] = offset + 53 + hash_table[new_id] * 53;
			else
				hand_ranks[id_index + new_card] = offset; // < 9 cards and id is not valid
		}
	}

}

// int generate_handranks(int *hand_ranks)
int generate_handranks(std::vector<int> &hand_ranks)
{
	std::vector<int64_t> id_fs, id_fr1, id_fr2, id_fr3, id_fr4, id_nf;

	std::cout << "\n====== PHASE 1 (GENERATE IDS) ======";

	std::cout << "\n\n>> IDs for flush suits... \n";
	generate_ids(100e3, id_fs, add_card_to_id_flush_suits);

	std::cout << "\n\n>> IDs for flush ranks (suit #1)... \n";	
	generate_ids(10e6, id_fr1, add_card_to_id_flush_ranks_1);

	std::cout << "\n\n>> IDs for flush ranks (suit #2)... \n";	
	generate_ids(10e6, id_fr2, add_card_to_id_flush_ranks_2);

	std::cout << "\n\n>> IDs for flush ranks (suit #3)... \n";	
	generate_ids(10e6, id_fr3, add_card_to_id_flush_ranks_3);

	std::cout << "\n\n>> IDs for flush ranks (suit #4)... \n";	
	generate_ids(10e6, id_fr4, add_card_to_id_flush_ranks_4);

	std::cout << "\n\n>> IDs for non-flush hands... \n";	
	generate_ids(100e6, id_nf, add_card_to_id_no_flush);

	int n_fs = id_fs.size(), n_fr1 = id_fr1.size(), n_fr2 = id_fr2.size(),
		n_fr3 = id_fr3.size(), n_fr4 = id_fr4.size(), n_nf = id_nf.size();

	std::cout << "\n\n\n====== PHASE 2 (PROCESS IDS) ======";

	int offset_fs = 0, offset_fr1, offset_fr2, offset_fr3, 
		offset_fr4, offset_nf, max_rank;
	offset_fr1 = offset_fs + 53 + n_fs * 53;
	offset_fr2 = offset_fr1 + 53 + n_fr1 * 53;
	offset_fr3 = offset_fr2 + 53 + n_fr2 * 53;
	offset_fr4 = offset_fr3 + 53 + n_fr3 * 53;
	offset_nf = offset_fr4 + 53 + n_fr4 * 53;
	max_rank = offset_nf + 53 + n_nf * 53;

	std::cout << "\n\nMAX_RANK = " << max_rank << "\n";
	std::vector<int>(max_rank, 0).swap(hand_ranks);

	int i, num_cards, new_card, new_index, id_index;
	int64_t id, new_id;

	std::cout << "\n\nEvaluating flush suits...\n";
	std::tr1::unordered_map<int, int> map_fs;
	map_fs.insert(std::tr1::unordered_map<int, int>::value_type(-1, offset_nf));
	map_fs.insert(std::tr1::unordered_map<int, int>::value_type(1, offset_fr1));
	map_fs.insert(std::tr1::unordered_map<int, int>::value_type(2, offset_fr2));
	map_fs.insert(std::tr1::unordered_map<int, int>::value_type(3, offset_fr3));
	map_fs.insert(std::tr1::unordered_map<int, int>::value_type(4, offset_fr4));
	process_ids(id_fs, offset_fs, offset_nf, hand_ranks,
		add_card_to_id_flush_suits, eval_flush_suits, map_fs);

	std::cout << "\n\nEvaluating flush ranks (suit #1)...\n";
	std::tr1::unordered_map<int, int> map_fr1;
	map_fr1.insert(std::tr1::unordered_map<int, int>::value_type(-1, offset_fr1));
	process_ids(id_fr1, offset_fr1, 0, hand_ranks,
		add_card_to_id_flush_ranks_1, eval_flush_ranks, map_fr1);

	std::cout << "\n\nEvaluating flush ranks (suit #2)...\n";
	std::tr1::unordered_map<int, int> map_fr2;
	map_fr2.insert(std::tr1::unordered_map<int, int>::value_type(-1, offset_fr2));
	process_ids(id_fr2, offset_fr2, 0, hand_ranks,
		add_card_to_id_flush_ranks_2, eval_flush_ranks, map_fr2);

	std::cout << "\n\nEvaluating flush ranks (suit #3)...\n";
	std::tr1::unordered_map<int, int> map_fr3;
	map_fr3.insert(std::tr1::unordered_map<int, int>::value_type(-1, offset_fr3));
	process_ids(id_fr3, offset_fr3, 0, hand_ranks,
		add_card_to_id_flush_ranks_3, eval_flush_ranks, map_fr3);

	std::cout << "\n\nEvaluating flush ranks (suit #4)...\n";
	std::tr1::unordered_map<int, int> map_fr4;
	map_fr4.insert(std::tr1::unordered_map<int, int>::value_type(-1, offset_fr4));
	process_ids(id_fr4, offset_fr4, 0, hand_ranks,
		add_card_to_id_flush_ranks_4, eval_flush_ranks, map_fr4);

	std::cout << "\n\nEvaluating non-flush hands...\n";
	std::tr1::unordered_map<int, int> map_nf;
	map_nf.insert(std::tr1::unordered_map<int, int>::value_type(-1, offset_nf));
	process_ids(id_nf, offset_nf, 0, hand_ranks,
		add_card_to_id_no_flush, eval_no_flush, map_nf);

	// hand_ranks.resize(max_rank);

	std::cout << "\n\nDone.\n";

	return max_rank;
}

void save_handranks(std::vector<int> hand_ranks, const char *filename)
{
	int *hr = &hand_ranks[0];
	std::cout << "\n\nSaving hand ranks to \"" << filename << "\"...";
	std::ofstream out(filename, std::ios::out | std::ios::binary);
	if (!out)
		std::cout << "\tError opening file.\n";
	else
	{
		int header[1];
		header[0] = hand_ranks.size(); // store the size in the first 4 bytes
		out.write(reinterpret_cast<const char *>(header), sizeof(int));
		out.write(reinterpret_cast<const char *>((int *) &hand_ranks[0]), 
			sizeof(int) * hand_ranks.size());
		std::cout << "\t" << hand_ranks.size() << " hand ranks written (+header).\n";
	}
}

// #define NUM_RANKS_NEW 373250539

int *load_new_handranks(const char *filename)
{
	FILE *f = fopen(filename, "rb");
	if (f)
	{
		int size = 0;
		fread(&size, sizeof(int), 1, f); // the size is stored in the first 4 bytes
		int *hand_ranks = (int *) malloc(size * sizeof(int));
		load_file((char *) hand_ranks, sizeof(int), size, f);
		fclose(f);
		return hand_ranks;
	}
	else
		return NULL;
}

#define NUM_RANKS_OLD 32487834

int *load_old_handranks(const char *filename)
{
	int *hand_ranks = (int *) malloc(NUM_RANKS_OLD * sizeof(int));
	FILE *f = fopen(filename, "rb");
	if (f)
	{
		load_file((char *) hand_ranks, sizeof(int), NUM_RANKS_OLD, f);
		fclose(f);
		return hand_ranks;
	}
	else
		return NULL;
}

void test_all_handranks(const char *filename)
{
	int *HR_new = load_new_handranks(filename);
	int *HR_old = load_old_handranks("HandRanks.dat");

	int c[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
	int64_t n = 0;
	int64_t N[3] = {133784560LL, 752538150LL,
		3679075400LL}; // C(52, 7), C(52, 8), C(52, 9)
	int min0[3] = {0,  0,  1};
	int max0[3] = {0,  0,  52};
	int min1[3] = {0,  1,  1};
	int max1[3] = {0,  52, 52};
	int n_board_perms[3] = {1, 4, 10};
	int board_paths[10];

	for (int k = 0; k < 3; k++)
	{
		std::cout << "\nChecking all " << (7 + k) << "-card sorted combinations...\n";
		n = 0;

		for (c[0] = min0[k]; c[0] <= max0[k]; c[0]++)
		{
			int fs0 = HR_new[53 + c[0]]; // flush suit
			int snf0 = HR_new[HR_new[0] + 53 + c[0]]; // score no flush
			for (c[1] = (min1[k] == 0) ? 0 : (c[0] + 1); c[1] <= max1[k]; c[1]++)
			{
				int fs1 = HR_new[fs0 + c[1]];
				int snf1 = HR_new[snf0 + c[1]];
				for (c[2] = c[1] + 1; c[2] <= 52; c[2]++)
				{
					int fs2 = HR_new[fs1 + c[2]];
					int snf2 = HR_new[snf1 + c[2]];
					for (c[3] = c[2] + 1; c[3] <= 52; c[3]++)
					{
						int fs3 = HR_new[fs2 + c[3]];
						int snf3 = HR_new[snf2 + c[3]];
						for (c[4] = c[3] + 1; c[4] <= 52; c[4]++)
						{
							int fs4 = HR_new[fs3 + c[4]];
							int snf4 = HR_new[snf3 + c[4]];

							for (int nb = 0; nb < n_board_perms[k]; nb++)
								board_paths[nb] = HR_old[HR_old[HR_old[53 + 
									c[(2 - k) + board_perms[nb][0]]] + 
									c[(2 - k) + board_perms[nb][1]]] +
									c[(2 - k) + board_perms[nb][2]]];

							for (c[5] = c[4] + 1; c[5] <= 52; c[5]++)
							{
								int fs5 = HR_new[fs4 + c[5]];
								int snf5 = HR_new[snf4 + c[5]];
								for (c[6] = c[5] + 1; c[6] <= 52; c[6]++)
								{
									int fs6 = HR_new[fs5 + c[6]];
									int snf6 = HR_new[snf5 + c[6]];
									for (c[7] = c[6] + 1; c[7] <= 52; c[7]++)
									{
										int fs7 = HR_new[fs6 + c[7]];
										int snf7 = HR_new[snf6 + c[7]];
										for (c[8] = c[7] + 1; c[8] <= 52; c[8]++)
										{
											int fs8 = HR_new[fs7 + c[8]];
											int snf8 = HR_new[snf7 + c[8]];
											// 139,177,470 - nf
											if (fs8 < 130000000)
											{
												int score_flush = fs8 + 53;
												for (int i = 0; i < 9; i++)
													score_flush = HR_new[score_flush + c[i]];
												snf8 = MAX(snf8, score_flush);
											}
											int score_new = snf8;

											int score_old = 0;
											for (int np = 0; np < n_pocket_perms; np++)
												for (int nb = 0; nb < n_board_perms[k]; nb++)
													score_old = MAX(score_old, 
														(HR_old[HR_old[HR_old[board_paths[nb] + 
														c[5 + pocket_perms[np][0]]] + 
														c[5 + pocket_perms[np][1]]]]));

											n++;
											if (score_new != score_old)
											{
												std::cout << "\n" << "(" << c[0];
												for (int i = 1; i < 9; i++)
													std::cout << ", " << c[i];
												std::cout << "): old = " << score_old <<
													", new = " << score_new << "\n";
												goto fail;
											}
											else if ((n % 1000LL) == 0)
											{
												std::cout << "\r\t" << n << " / " << N[k] << 
													" combinations verified";
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
		std::cout << "\r\t" << n << " / " << N[k] << " combinations verified";
	}

	std::cout << "\n\nAll combinations verified successfully.\n\n";
	std::cout << "\nGreat success.\n";
	return;

fail: 
	std::cout << "\nEpic fail.\n";
}

void test_handranks(const char *filename)
{
	int *HR = load_new_handranks(filename);

	//[--, --, '2c', '3c', '4c', '4d', '4h', '5c', '6c']
	int cards[9] = {0, 0, 1, 5, 9, 10, 11, 13, 17},
		n_cards = 9;

	int offset = 53;
	std::cout << "offset: " << offset << "\n";
	for (int i = 0; i < n_cards; i++)
	{
		offset = HR[offset + cards[i]];
		std::cout << "card = " << cards[i] << " --> offset = " << offset << "\n";
	}

	int rank = offset + 53;
	for (int i = 0; i < n_cards; i++)
	{
		rank = HR[rank + cards[i]];
		std::cout << "\tcard = " << cards[i] << " --> rank = " << rank << "\n";
	}
	std::cout << "Hand rank: " << rank << "\n";	
}

void test_flush_suits()
{
	//[--, --, '2c', '3c', '4c', '4d', '4h', '5c', '6c']
	int cards[9] = {0, 0, 1, 5, 9, 10, 11, 13, 17};
	const char *suits[] = {"n_a", "c", "d", "h", "s"};

	std::vector<int64_t> id_fs;
	std::vector<int> hand_ranks;
	std::cout << "\n\n>> IDs for flush suits... \n";
	generate_ids(100e3, id_fs, add_card_to_id_flush_suits);
	int n_fs = id_fs.size(), offset_fs = 0, max_rank = offset_fs + 53 + n_fs * 53;
	std::vector<int>(max_rank, 0).swap(hand_ranks);
	std::cout << "\n\nEvaluating flush suits...\n";
	int offset_nf = 500000000;
	process_ids(id_fs, offset_fs, offset_nf, hand_ranks,
		add_card_to_id_flush_suits, eval_flush_suits);
	std::replace(hand_ranks.begin(), hand_ranks.end(), -1, offset_nf);
	std::replace(hand_ranks.begin(), hand_ranks.end(), 1, 100000000);
	std::replace(hand_ranks.begin(), hand_ranks.end(), 2, 200000000);
	std::replace(hand_ranks.begin(), hand_ranks.end(), 3, 300000000);
	std::replace(hand_ranks.begin(), hand_ranks.end(), 4, 400000000);
	std::cout << "\n\n";
	std::tr1::unordered_map<int64_t, int> hash_table;
	for (int i = 0; i < n_fs; i++)
		hash_table.insert(std::tr1::unordered_map<int64_t, int>::value_type(id_fs[i], i));

	int64_t id = 0LL;
	int index = 53;
	std::cout << "Starting:\n\tid = 0, index = 53\n";
	for (int i = 0; i < 9; i++)
	{
		id = add_card_to_id_flush_suits(id, cards[i]);

		std::cout << "Adding card #" << (i + 1) << ": id -> " << id;
		if (i < 8)
		{
			index = hand_ranks[index + cards[i]];
			int id_index = (index - 53) / 53;
			std::cout << ", index -> " << index << ", id_fs[id_index] = " << 
				id_fs[id_index];
		}
		else
		{
			std::cout << ", value = " << hand_ranks[index + cards[i]];
		}
		std::cout << "\n";
		if (!id)
			break;
		int pocket[4] = {0, 0, 0, 0}, board[5] = {0, 0, 0, 0, 0}, 
			n_pocket = 0, n_board = 0;
		unpack64(id, pocket, board, n_pocket, n_board);
		std::cout << "\tPocket:";
		for (int i = 0; i < n_pocket; i++)
			std::cout << " " << pocket[i] << "(" << suits[pocket[i]] << ")";
		std::cout << "\n\tBoard:";
		for (int i = 0; i < n_board; i++)
			std::cout << " " << board[i] << "(" << suits[board[i]] << ")";
		std::cout << "\n";
	}

	std::cout << "Eval flush suits: " << eval_flush_suits(id) << "\n\n";
}

void test_straight_flush()
{
	//[--, --, '2c', '3c', '4c', '4d', '4h', '5c', '6c']
	int cards[9] = {0, 0, 1, 5, 9, 10, 11, 13, 17};
	int64_t id = 0LL;
	for (int i = 0; i < 9; i++)
		id = add_card_to_id_flush_ranks_1(id, cards[i]);
	int pocket[4], board[5], n_pocket, n_board;
	unpack64(id, pocket, board, n_pocket, n_board);
	std::cout << "\tPocket:";
	for (int i = 0; i < n_pocket; i++)
		std::cout << " " << pocket[i] << "";
	std::cout << "\n\tBoard:";
	for (int i = 0; i < n_board; i++)
		std::cout << " " << board[i] << "";
	std::cout << "\n";
	int q = (1 << (pocket[0] - 2)) | (1 << (pocket[1] - 2)) |
		(1 << (board[0] - 2)) | (1 << (board[1] - 2)) | (1 << (board[2] - 2));
	std::cout << "eval: " << eval_flush_ranks(id) << "\n";;
}

void test_flush_ranks_1()
{
	std::vector<int64_t> id_fr1;
	std::vector<int> hand_ranks;
	std::cout << "\n\n>> IDs for flush ranks (suit #1)... \n";
	generate_ids(10e6, id_fr1, add_card_to_id_flush_ranks_1);
	int n_fr1 = id_fr1.size(), offset_fr1 = 53 + 2485 * 53, 
		max_rank = offset_fr1 + 53 + n_fr1 * 53;
	std::vector<int>(max_rank, 0).swap(hand_ranks);
	std::cout << "\n\nEvaluating flush ranks (suit #1)...\n";
	process_ids(id_fr1, offset_fr1, 0, hand_ranks,
		add_card_to_id_flush_ranks_1, eval_flush_ranks);
	std::cout << "\n\n";
	std::tr1::unordered_map<int64_t, int> hash_table;
	for (int i = 0; i < n_fr1; i++)
		hash_table.insert(std::tr1::unordered_map<int64_t, int>::value_type(id_fr1[i], i));
}

void test_no_flush()
{
	int64_t id = 13472513442443LL;
	id = add_card_to_id_no_flush(id, 1);
	int pocket[4], board[5], n_pocket, n_board;
	unpack64(id, pocket, board, n_pocket, n_board);
	std::cout << "\tPocket:";
	for (int i = 0; i < n_pocket; i++)
		std::cout << " " << pocket[i] << "";
	std::cout << "\n\tBoard:";
	for (int i = 0; i < n_board; i++)
		std::cout << " " << board[i] << "";
	std::cout << "\n";
	std::cout << "eval: " << eval_no_flush(id) << "\n";
}

void test_flush_hand()
{
	//[--, --, '2c', '3c', '4c', '4d', '4h', '5c', '6c']
	int cards[9] = {0, 0, 1, 5, 9, 10, 11, 13, 17};
	int64_t id = 0LL;

	for (int i = 0; i < 9; i++)
	{
		std::cout << "\n=== Adding card " << cards[i] <<  " ===\n";
		id = add_card_to_id_flush_ranks_1(id, cards[i]);
		std::cout << "\n";
	}
	int pocket[4], board[5], n_pocket, n_board;
	unpack64(id, pocket, board, n_pocket, n_board);
	std::cout << "\tPocket:";
	for (int i = 0; i < n_pocket; i++)
		std::cout << " " << pocket[i] << "";
	std::cout << "\n\tBoard:";
	for (int i = 0; i < n_board; i++)
		std::cout << " " << board[i] << "";
	std::cout << "\n";
	std::cout << "eval: " << eval_flush_ranks(id) << "\n";
	std::cout << "id: " << id << "\n";
}

bool vec9_equal(const std::vector<int> &v1, const std::vector<int> &v2)
{
	for (int i = 0; i < 9; i++)
		if (v1[i] != v2[i])
			return false;
	return true;
}

void prune_flush_ranks(const char *filename)
{
	int offset_fr1 = 0;
	std::vector<int> hand_ranks;
	std::vector<int>(500e6, 0).swap(hand_ranks);
	std::vector<int64_t> id_fr1;
	std::cout << "\n\n>> IDs for flush ranks (suit #1)... \n";	
	generate_ids(200e6, id_fr1, add_card_to_id_no_flush);
	// generate_ids(10e6, id_fr1, add_card_to_id_flush_ranks_1);

	std::cout << "\n\nEvaluating flush ranks (suit #1)...\n";
	std::tr1::unordered_map<int, int> map_fr1;
	map_fr1.insert(std::tr1::unordered_map<int, int>::value_type(-1, offset_fr1));
	process_ids(id_fr1, offset_fr1, 0, hand_ranks,
		add_card_to_id_no_flush, eval_no_flush, map_fr1);
		// add_card_to_id_flush_ranks_1, eval_flush_ranks, map_fr1);
	std::cout << "\n";

	std::cout << "\nCreating paths graph...\n";
	std::vector< std::vector<int> > paths;
	for (int i = 0; i < id_fr1.size(); i++)
	{
		std::cout << "\r" << i + 1 << " / " << id_fr1.size() << "...";
		int64_t id = id_fr1[i];
		std::vector<int> v;
		for (int j = 0; j <= 52; j++)
			v.push_back(hand_ranks[offset_fr1 + 53 + i * 53 + j]);
		paths.push_back(v);
	}

	// std::vector<int>::iterator duplicates = 
	std::vector< std::vector<int> >::iterator duplicates =
		std::unique(paths.begin(), paths.end(), vec9_equal);
	std::cout << "\n" << paths.size() << " --> " << (duplicates - paths.begin()) << "\n";
}

struct commas_locale : std::numpunct<char> 
{ 
	char do_thousands_sep() const { return ','; } 
	std::string do_grouping() const { return "\3"; }
};

int main()
{
	std::vector<int> hand_ranks;

	std::cout.imbue(std::locale(std::locale(), new commas_locale));

	// generate_handranks(hand_ranks);
	// save_handranks(hand_ranks, "hr9_cpp4.dat");
	// test_handranks("hr9_cpp4.dat");	
	test_all_handranks("hr9_cpp4.dat");

	// prune_flush_ranks("hr9_cpp.dat");

	// test_flush_suits();
	// test_straight_flush();
	// test_flush_hand();
	// test_flush_ranks_1();
	// test_no_flush();
}
