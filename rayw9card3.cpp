#include <iostream>
#include <fstream>
#include <algorithm>
#include <vector>
#include <iomanip>
#include <locale>
#include <exception>
#include <tr1/unordered_map>

#include "arrays.h"
#include "load_file.h"

#define MAX(x, y) 				((x) > (y) ? (x) : (y))
#define MIN(x, y) 				((x) < (y) ? (x) : (y))
#define ANY_CARD 				1
#define SWAP_LESS(A, I, J) 		{if (A[I] < A[J]) {A[I] ^= A[J]; A[J] ^= A[I]; A[I] ^= A[J];}}
#define SWAP_GREATER(A, I, J) 	{if (A[I] > A[J]) {A[I] ^= A[J]; A[J] ^= A[I]; A[I] ^= A[J];}}

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

template <class T>
bool is_sorted(std::vector<T> vec)
{
	return (std::adjacent_find(vec.begin(), vec.end(), std::greater<T>()) == vec.end());	
}

// this shit is slow, use hash tables (tr1::unordered_map) instead
int find_first(std::vector<int64_t> vec, int64_t value)
{
	// return equal_range(vec.begin(), vec.end(), value).first - vec.begin();
	return (std::lower_bound(vec.begin(), vec.end(), value) - vec.begin());
}

void add_card(int new_card, int *pocket, int *board, int &n_pocket, int &n_board)
{
	if (n_pocket < 4)
		pocket[n_pocket++] = new_card;
	else
		board[n_board++] = new_card;
}

void unpack64(int64_t id, int *pocket, int *board, int &n_pocket, int &n_board)
{
	n_pocket = 0;
	n_board = 0;
	int card;
	for (int i = 0; i < 9; i++)
	{
		if ((card = (int) ((id >> (7 * i)) & 0x7F)) != 0)
		{
			if (i < 4)
				pocket[n_pocket++] = card;
			else
				board[n_board++] = card;
		}
	}
}

int64_t pack64(int *pocket, int *board)
{
	std::sort(pocket, pocket + 4);
	std::sort(board, board + 5);
	std::reverse(pocket, pocket + 4);
	std::reverse(board, board + 5);

	return ((int64_t) pocket[0] +
		 ((int64_t) pocket[1] << 7) +
		 ((int64_t) pocket[2] << 14) + 
		 ((int64_t) pocket[3] << 21) +
		 ((int64_t) board[0] << 28) +
		 ((int64_t) board[1] << 35) +
		 ((int64_t) board[2] << 42) +
		 ((int64_t) board[3] << 49) +
		 ((int64_t) board[4] << 56));    
}

int64_t add_card_to_id_flush_suits(int64_t id, int new_card)
{
	int pocket[4] = {0, 0, 0, 0}, board[5] = {0, 0, 0, 0, 0}, 
		n_pocket = 0, n_board = 0;
	new_card = ((new_card - 1) & 3) + 1;
	unpack64(id, pocket, board, n_pocket, n_board);
	add_card(new_card, pocket, board, n_pocket, n_board);
	return pack64(pocket, board);
}

int eval_flush_suits(int64_t id)
{
	int pocket[4] = {0, 0, 0, 0}, board[5] = {0, 0, 0, 0, 0}, 
		n_pocket = 0, n_board = 0, n_suit_pocket[5] = {0, 0, 0, 0, 0},
		n_suit_board[5] = {0, 0, 0, 0, 0};
	unpack64(id, pocket, board, n_pocket, n_board);
	for (int i = 0; i < n_pocket; i++)
		n_suit_pocket[pocket[i]] = MIN(n_suit_pocket[pocket[i]] + 1, 2);
	for (int i = 0; i < n_board; i++)
		n_suit_board[board[i]] = MIN(n_suit_board[board[i]] + 1, 3);
	for (int suit = 1; suit <= 4; suit++)
	{
		// std::cout << "\n" << suit << ": nsp = " << n_suit_pocket[suit] <<
		// 	", nsb = " << n_suit_board[suit];
		if ((n_suit_pocket[suit] + n_suit_board[suit]) >= 5)
			return suit;
	}
	return -1;
}

int64_t add_card_to_id_flush_ranks(int64_t id, int new_card, int flush_suit)
{
	int pocket[4] = {0, 0, 0, 0}, board[5] = {0, 0, 0, 0, 0}, 
		n_pocket = 0, n_board = 0, nsp = 0, nsb = 0, i;

	// the rank is 2-14 if suited or 1 otherwise (for sorting purposes)
	new_card = ((((new_card - 1) & 3) + 1) == flush_suit) ? 
		(2 + ((new_card - 1) >> 2) & 0xF) : ANY_CARD;

	unpack64(id, pocket, board, n_pocket, n_board);

	for (i = 0; i < n_pocket; i++)
		if (pocket[i] != ANY_CARD && pocket[i] == new_card) 
			return 0;
	for (i = 0; i < n_board; i++)
		if (board[i] != ANY_CARD && board[i] == new_card)
			return 0;

	add_card(new_card, pocket, board, n_pocket, n_board);

	for (i = 0; i < n_pocket; i++)
		if (pocket[i] != ANY_CARD && pocket[i] != 0) 
			nsp++;
	for (i = 0; i < n_board; i++)
		if (board[i] != ANY_CARD && board[i] != 0)
			nsb++;

	// std::sort(pocket, pocket + 4);
	// std::sort(board, board + 5);
	// std::reverse(pocket, pocket + 4);
	// std::reverse(board, board + 5);

	bool debug = false;

	// if (n_pocket == 4 && n_board == 2 && pocket[0] == 4 && pocket[1] == 3 &&
	// 	pocket[2] == ANY_CARD && pocket[3] == ANY_CARD && board[0] == ANY_CARD &&
	// 	board[1] == ANY_CARD)
	// {
	// 	debug = true;
	// }

	if (debug) std::cout << "\n4321+11: nsp = " << nsp << ", nsb = " << nsb;
	if (debug) std::cout << "\nn_pocket = " << n_pocket << ", n_board = " << n_board;

	// if (pack64(pocket, board) == 4432676815236LL)
		// std::cout << "\n4321+111: nsp = " << nsp << ", nsb = " << nsb << "\n";

	if (n_pocket >= 4 && nsp < 2)
		return 0;

	if (n_pocket >= 4 && n_board >= 2)
		if ((5 - MAX(nsp, 2) - MAX(nsb, 3)) > (9 - n_pocket - n_board))
			return 0;

	if (debug) std::cout << "\n\tnon-zero...\n";

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
	int pocket[4] = {0, 0, 0, 0}, board[5] = {0, 0, 0, 0, 0}, 
		n_pocket = 0, n_board = 0;
	unpack64(id, pocket, board, n_pocket, n_board);
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

	// int q = (1 << (pocket[0] - 2)) | (1 << (pocket[1] - 2)) |
	// 	(1 << (board[0] - 2)) | (1 << (board[1] - 2)) | (1 << (board[2] - 2));
	// return cactus_to_ray(flushes[q]);
}

int64_t add_card_to_id_no_flush(int64_t id, int new_card)
{
	int pocket[4] = {0, 0, 0, 0}, board[5] = {0, 0, 0, 0, 0},
		n_rank[14] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		n_pocket = 0, n_board = 0, i;
	new_card = 1 + ((new_card - 1) >> 2) & 0xF;
	unpack64(id, pocket, board, n_pocket, n_board);
	for (i = 0; i < n_pocket; i++)
		n_rank[pocket[i]]++;
	for (i = 0; i < n_board; i++)
		n_rank[board[i]]++;
	add_card(new_card, pocket, board, n_pocket, n_board);
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

	// rank--;
	// return primes[rank] | (rank << 8) | (1 << (suit + 11)) | (1 << (16 + rank));
}

int cactus_findit(int key)
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
    std::cout << "\ncactus_findit(): no match found; key = " << key << "\n";
    return -1;
}

int eval_cactus(int c1, int c2, int c3, int c4, int c5)
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
	const int pocket_perms[2][6] = {{0, 0, 0, 1, 1, 2}, {1, 2, 3, 2, 3, 3}};
	const int n_pocket_perms = 6;
	const int board_perms[10][3] = {
		{0, 1, 2}, // 3, 4, 5
		{0, 1, 3}, {0, 2, 3}, {1, 2, 3}, // 4, 5
		{0, 1, 4}, {0, 2, 4}, {0, 3, 4}, {1, 2, 4}, {1, 3, 4}, {2, 3, 4}}; // 5

	int pocket[4] = {0, 0, 0, 0}, board[5] = {0, 0, 0, 0, 0}, 
		n_pocket = 0, n_board = 0;
	unpack64(id, pocket, board, n_pocket, n_board);

	int n = n_pocket + n_board;
	int n_board_perms = n == 9 ? 10 : n == 8 ? 4 : n == 7 ? 1 : -1;
	if (n_pocket < 4 || n_board < 3)
	{	
		std::cout << "\neval_no_flush() encountered invalid # of cards, shouldn't happen...";
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
			q = eval_cactus(pocket[pocket_perms[0][np]],
				pocket[pocket_perms[1][np]],
				board[board_perms[nb][0]],
				board[board_perms[nb][1]],
				board[board_perms[nb][2]]);
			if (q < best)
				best = q;
		}
	}
	return cactus_to_ray(best);
}

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
		for (int i = 0; i < id_queue_1.size(); i++)
		{
			std::cout << "\r\t" << "Processing ID " << i + 1 << " / " <<
				id_queue_1.size() << "...";
			int64_t id = id_queue_1[i];

			for (int new_card = 1; new_card <= 52; new_card++)
				if ((new_id = (*add_card_to_id)(id, new_card)) != 0)
					id_queue_2.push_back(new_id);
		}
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

void process_ids(std::vector<int64_t> ids, int offset, int offset_value,
	std::vector<int> &hand_ranks, int64_t (*add_card_to_id)(int64_t, int),
	int (*eval_id)(int64_t))
{
	// 0-52: loop with exit to 0
	// 53-105: normal start
	// 106+: normal

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
		// bool debug = (offset + 53 + i * 53) == 93545; // for flush suits
		// bool debug = (i == 228233); // for flush ranks 1
		bool debug = false;

		id = ids[i];
		std::cout << "\r\tProcessing ID " << i + 1 << " out of " << n << " (" << id << ")..." ;
		id_index = offset + 53 + i * 53;
		num_cards = count_cards(id);
		hand_ranks[id_index] = offset; // safety backup

		if (debug)
		{
			std::cout << "\ni = " << i << ", id = " << id << 
				", num_cards = " << num_cards << "\n"; 
		}

		if (num_cards == 7 || num_cards == 8)
		{
			if (debug)
			{
				std::cout << "evaluating id: ";
				std::cout << (*eval_id)(id) << "\n";
			}
			int value = (*eval_id)(id);
			hand_ranks[id_index] = (value != -1) ? value : offset;
		}

		for (new_card = 1; new_card <= 52; new_card++)
		{			
			new_id = (*add_card_to_id)(id, new_card);
			// if (debug)
			// {
			// 	std::cout << "adding card " << new_card << " to id " << id << "...\n";
			// 	std::cout << "\tnew_id = " << new_id << "\n";
			// 	std::cout << "\tnum_cards: " << count_cards(id) << " -> " << count_cards(new_id) << "\n";
			// }
			if (new_id && ((num_cards + 1) == 9))
			{
				hand_ranks[id_index + new_card] = (*eval_id)(new_id);
				if (debug) std::cout << "\t1\n";
			}
			else if (new_id)
			{
				hand_ranks[id_index + new_card] = offset + 53 + hash_table[new_id] * 53;
				// if (debug) std::cout << "\t2: offset = " << offset << ", " <<
				// 	" num_cards = " << num_cards << ", hash_table[new_id] = " << 
				// 	hash_table[new_id] << "\n";
			}
			else
			{
				hand_ranks[id_index + new_card] = offset; // TODO: QUESTIONABLE
				// if (debug) std::cout << "\t3\n";
			}
		}
	}

}

void generate_handranks(std::vector<int> &hand_ranks)
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
	generate_ids(50e6, id_nf, add_card_to_id_no_flush);

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

	std::vector<int>(max_rank, 0).swap(hand_ranks);

	int i, num_cards, new_card, new_index, id_index;
	int64_t id, new_id;

	std::cout << "\n\nEvaluating flush suits...\n";
	process_ids(id_fs, offset_fs, offset_nf, hand_ranks,
		add_card_to_id_flush_suits, eval_flush_suits);
	std::replace(hand_ranks.begin(), hand_ranks.begin() + offset_fr1 - 1, -1, offset_nf);
	std::replace(hand_ranks.begin(), hand_ranks.begin() + offset_fr1 - 1, 1, offset_fr1);
	std::replace(hand_ranks.begin(), hand_ranks.begin() + offset_fr1 - 1, 2, offset_fr2);
	std::replace(hand_ranks.begin(), hand_ranks.begin() + offset_fr1 - 1, 3, offset_fr3);
	std::replace(hand_ranks.begin(), hand_ranks.begin() + offset_fr1 - 1, 4, offset_fr4);

	std::cout << "\n\nEvaluating flush ranks (suit #1)...\n";
	try {
		process_ids(id_fr1, offset_fr1, 0, hand_ranks,
			add_card_to_id_flush_ranks_1, eval_flush_ranks);
	} catch (std::exception &e) {
		std::cout << e.what() << "\n";
	}

	std::cout << "\n\nEvaluating flush ranks (suit #2)...\n";
	process_ids(id_fr2, offset_fr2, 0, hand_ranks,
		add_card_to_id_flush_ranks_2, eval_flush_ranks);

	std::cout << "\n\nEvaluating flush ranks (suit #3)...\n";
	process_ids(id_fr3, offset_fr3, 0, hand_ranks,
		add_card_to_id_flush_ranks_3, eval_flush_ranks);

	std::cout << "\n\nEvaluating flush ranks (suit #4)...\n";
	process_ids(id_fr4, offset_fr4, 0, hand_ranks,
		add_card_to_id_flush_ranks_4, eval_flush_ranks);

	std::cout << "\n\nEvaluating non-flush hands...\n";
	process_ids(id_nf, offset_nf, 0, hand_ranks,
		add_card_to_id_no_flush, eval_no_flush);

	std::cout << "\n\nDone.\n";
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
		out.write(reinterpret_cast<const char *>((int *) &hand_ranks[0]), 
			sizeof(int) * hand_ranks.size());
		std::cout << "\t" << hand_ranks.size() << " hand ranks written.\n";
	}
}

#define NUM_RANKS_NEW 347382723

int *load_new_handranks(const char *filename)
{
	int *hand_ranks = (int *) malloc(NUM_RANKS_NEW * sizeof(int));
	FILE *f = fopen(filename, "rb");
	if (f)
	{
		load_file((char *) hand_ranks, sizeof(int), NUM_RANKS_NEW, f);
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

int eval_5cards(int c1, int c2, int c3, int c4, int c5)
{
    int q;
    int s;
    q = (c1 | c2 | c3 | c4 | c5) >> 16;
    if (c1 & c2 & c3 & c4 & c5 & 0xF000) // check for flushes and straight-flushes
		return ( flushes[q] );
    s = unique5[q]; // check for straights and high-card hands
    if (s) 
    	return s;
    q = (c1 & 0xFF) * (c2 & 0xFF) * (c3 & 0xFF) * (c4 & 0xFF) * (c5 & 0xFF);
    q = cactus_findit(q);
    return values[q];
}

int eval_5hand(int *hand)
{
    int c1, c2, c3, c4, c5;
    c1 = *hand++; c2 = *hand++; c3 = *hand++; c4 = *hand++; c5 = *hand;
    return eval_5cards(c1, c2, c3, c4, c5);
}

int eval_789(int *hand_ranks, int *cards, int n)
{
	const int pocket_perms[2][6] = {{0, 0, 0, 1, 1, 2}, {1, 2, 3, 2, 3, 3}};
	const int n_pocket_perms = 6;
	const int board_perms[10][3] = {
		{0, 1, 2}, // 3, 4, 5
		{0, 1, 3}, {0, 2, 3}, {1, 2, 3}, // 4, 5
		{0, 1, 4}, {0, 2, 4}, {0, 3, 4}, {1, 2, 4}, {1, 3, 4}, {2, 3, 4}}; // 5
	int n_board_perms = n == 9 ? 10 : n == 8 ? 4 : n == 7 ? 1 : -1;
	if (n_board_perms == -1)
		return -1;
	int np, nb, best = -1, score = 0;
	for (np = 0; np < n_pocket_perms; np++)
	{
		for (nb = 0; nb < n_board_perms; nb++)
		{
			int subhand[5];
			subhand[0] = cards[pocket_perms[0][np]];
			subhand[1] = cards[pocket_perms[1][np]];
			subhand[2] = cards[4 + board_perms[nb][0]];
			subhand[3] = cards[4 + board_perms[nb][1]];
			subhand[4] = cards[4 + board_perms[nb][2]];
			score = 53;
			for (int i = 0; i < 5; i++)
				score = hand_ranks[score + subhand[i]];
			score = hand_ranks[score];
			if (score > best)
				best = score;
		}
	}
	return best;
}


void test_all_handranks(const char *filename)
{
	int *HR_new = load_new_handranks(filename);
	int *HR_old = load_old_handranks("HandRanks.dat");

	int c[9];
	int64_t N = 52LL * 51LL * 50LL * 49LL * 48LL * 47LL * 46LL * 45LL * 44LL;
	int64_t n = 0;
	for (c[0] = 1; c[0] <= 52; c[0]++)
		for (c[1] = c[0] + 1; c[1] <= 52; c[1]++)
			for (c[2] = c[1] + 1; c[2] <= 52; c[2]++)
				for (c[3] = c[2] + 1; c[3] <= 52; c[3]++)
					for (c[4] = c[3] + 1; c[4] <= 52; c[4]++)
						for (c[5] = c[4] + 1; c[5] <= 52; c[5]++)
							for (c[6] = c[5] + 1; c[6] <= 52; c[6]++)
								for (c[7] = c[6] + 1; c[7] <= 52; c[7]++)
									for (c[8] = c[7] + 1; c[8] <= 52; c[8]++)
									{
										int offset = 53;
										for (int i = 0; i < 9; i++)
											offset = HR_new[offset + c[i]];
										int score_no_flush = 0;
										if (offset < 120000000)
										{
											score_no_flush = HR_new[0] + 53;
											for (int i = 0; i < 9; i++)
												score_no_flush = HR_new[score_no_flush + c[i]];
										}
										int score_new = offset + 53;
										for (int i = 0; i < 9; i++)
											score_new = HR_new[score_new + c[i]];
										score_new = MAX(score_new, score_no_flush);
										int score_old = eval_789(HR_old, c, 9);
										if (score_new == score_old)
											std::cout << "\r" << ++n << " / " << N << 
												" combinations verified";
										else
										{
											std::cout << "\n" << "(" << c[0];
											for (int i = 1; i < 9; i++)
												std::cout << ", " << c[i];
											std::cout << "): old = " << score_old <<
												", new = " << score_new << "\n";
											goto fail;
										}
									}
	std::cout << "\n\nAll combinations verified successfully.\n\n";

fail: 
	std::cout << "\nFail.\n";

}

void test_handranks(const char *filename)
{
	int *HR = load_new_handranks(filename);

	// ['2d', '2h', '2s', '3d', '3h', '4d', '5d', '6d', '7d']
	int cards[9] = {1, 2, 3, 5, 6, 9, 13, 17, 21};
	for (int i = 0; i < 9; i++)
		cards[i]++;

	int offset = 53;
	std::cout << "offset: " << offset << "\n";
	for (int i = 0; i < 9; i++)
	{
		offset = HR[offset + cards[i]];
		std::cout << "card = " << cards[i] << " --> offset = " << offset << "\n";
	}

	int rank = offset + 53;
	for (int i = 0; i < 9; i++)
		rank = HR[rank + cards[i]];
	std::cout << "Hand rank: " << rank << "\n";	
}

void test_flush_suits()
{
	// ['2c', '2d', '2h', '3c', '4c', '4d', '4h', '5c', '7c']
	int cards[9] = {1, 2, 3, 5, 9, 10, 11, 13, 21};
	const char *suits[] = {"n/a", "c", "d", "h", "s"};

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
	// ['2d', '2h', '2s', '3d', '3h', '4d', '5d', '6d', '7d']
	int cards[9] = {1, 2, 3, 5, 6, 9, 13, 17, 21};
	int64_t id = 0LL;
	for (int i = 0; i < 9; i++)
	{
		id = add_card_to_id_flush_ranks_1(id, cards[i]);
	}
	int pocket[4] = {0, 0, 0, 0}, board[5] = {0, 0, 0, 0, 0}, 
		n_pocket = 0, n_board = 0;
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
	int pocket[4] = {0, 0, 0, 0}, board[5] = {0, 0, 0, 0, 0}, 
		n_pocket = 0, n_board = 0;
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

void temp()
{
	// int64_t id = 145249988232905092LL;
	// int pocket[4] = {0, 0, 0, 0}, board[5] = {0, 0, 0, 0, 0}, 
	// 	n_pocket = 0, n_board = 0;
	// unpack64(id, pocket, board, n_pocket, n_board);

	int pocket[4] = {1, 2, 3, 4}, board[5] = {5, 6, 7, 8, 9},
		n_pocket = 4, n_board = 5;
	int64_t id = pack64(pocket, board);

	std::cout << "\tPocket:";
	for (int i = 0; i < n_pocket; i++)
		std::cout << " " << pocket[i] << "";
	std::cout << "\n\tBoard:";
	for (int i = 0; i < n_board; i++)
		std::cout << " " << board[i] << "";
	std::cout << "\n";
	std::cout << "eval: " << eval_flush_suits(id) << "\n";
}

void test_flush_hand()
{
	// ['2c', '2d', '2h', '3c', '4c', '4d', '4h', '5c', '7c']
	int cards[9] = {1, 2, 3, 5, 9, 10, 11, 13, 21};
	int64_t id = 0LL;
	for (int i = 0; i < 9; i++)
	{
		id = add_card_to_id_flush_ranks_1(id, cards[i]);
	}
	int pocket[4] = {0, 0, 0, 0}, board[5] = {0, 0, 0, 0, 0}, 
		n_pocket = 0, n_board = 0;
	unpack64(id, pocket, board, n_pocket, n_board);
	std::cout << "\tPocket:";
	for (int i = 0; i < n_pocket; i++)
		std::cout << " " << pocket[i] << "";
	std::cout << "\n\tBoard:";
	for (int i = 0; i < n_board; i++)
		std::cout << " " << board[i] << "";
	std::cout << "\n";
	std::cout << "eval: " << eval_flush_ranks(id) << "\n";
	int q = (1 << (2 - 2)) |
		(1 << (3 - 2)) |
		(1 << (4 - 2)) |
		(1 << (5 - 2)) |
		(1 << (7 - 2));
	std::cout << "direct: " << cactus_to_ray(flushes[q]) << "\n";
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

	// test_flush_suits();
	// test_straight_flush();

	// test_flush_hand();

	// temp();
	// test_flush_ranks_1();
	// test_no_flush();

	generate_handranks(hand_ranks);
	save_handranks(hand_ranks, "hr9_cpp.dat");

	test_all_handranks("hr9_cpp.dat");
	// test_handranks("hr9_cpp.dat");
}
