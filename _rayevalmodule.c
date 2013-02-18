#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <Python.h>

#define MAX_PLAYERS 10

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
		fread(_HR, sizeof(_HR), 1, f);
		fclose(f);
		return _HR;
	}
	else
		return NULL;
}

static int RAND_MAX_DIV_52[53];

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

void init_random_int_52(void)
{
	int i;
	for (i = 1; i <= 52; i++)
		RAND_MAX_DIV_52[i] = RAND_MAX / (i + 1);
	srand(mix(clock(), time(NULL), getpid()));
}

// generate a random integer from 0 to k
int random_int_52(int k) 
{
    int r;
	do 
	{
		r = rand() / RAND_MAX_DIV_52[k];
    } while (r > k);
    return r;
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

void random_sample_52_ross(int n, int k, int *out)
{
	// Ross algorithm modified to work in-place (C) Aldanor
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

int eval_monte_carlo_omaha(int N, int *board, int n_board, 
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

	unsigned int mask[52], cards[52], n_mask = 0, n_available, 
		i, available_cards[52], sample[52];

	memset(mask, 0, 52 * sizeof(unsigned int));
	memset(cards, 0, 52 * sizeof(unsigned int));
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
		unsigned int best_score = 0;
		unsigned int tied = 0;
		random_sample_52_ross(n_available, n_mask, sample);
		unsigned int j = 0, nb = 0, np = 0;
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

////////////////////////////////////////////////////////////////////////////////
//							PYTHON WRAPPERS
////////////////////////////////////////////////////////////////////////////////

#define RAISE_EXCEPTION(exception, message) {PyErr_SetString((exception), (message)); return NULL;}

static PyObject *_rayeval_seed(PyObject *self, PyObject *args)
{
	unsigned long seed;
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
	int iterations, n_board, n_pocket, n_players, i, pocket_size = 2,
		board[5], pocket[4 * MAX_PLAYERS], is_omaha = 0;
	double ev[MAX_PLAYERS];

	if (!PyArg_ParseTuple(args, "sOOi", &game, &py_board, &py_pocket, &iterations))
		return NULL;

	if (!strcmp(game, "omaha"))
		is_omaha = 1;
	else if (strcmp(game, "holdem"))
    	RAISE_EXCEPTION(PyExc_ValueError, "Game type must be holdem or omaha.");
    pocket_size += 2 * is_omaha;

  	if (!PyList_Check(py_board)) 
    	RAISE_EXCEPTION(PyExc_TypeError, "Board must be a list.");
  	if (!PyList_Check(py_pocket)) 
    	RAISE_EXCEPTION(PyExc_TypeError, "Pockets must be a list.");
    if (iterations <= 0)
    	RAISE_EXCEPTION(PyExc_ValueError, "Iterations must be a positive integer.");

    n_pocket = PyList_Size(py_pocket);
    n_board = PyList_Size(py_board);
    n_players = n_pocket / pocket_size;
    if (n_players * pocket_size != n_pocket)
    	RAISE_EXCEPTION(PyExc_ValueError, "Invalid number of pocket cards.");
    if (n_board != 5)
    	RAISE_EXCEPTION(PyExc_ValueError, "Board must contain five cards.");

    for (i = 0; i < n_pocket; i++)
    {
    	PyObject *item = PyList_GetItem(py_pocket, i);
    	if (!PyInt_Check(item))
	    	RAISE_EXCEPTION(PyExc_TypeError, "Pocket cards must be integers.");
    	pocket[i] = (int) PyInt_AsLong(item);
    	if ((pocket[i] < 0 || pocket[i] > 51) && pocket[i] != 255)
	    	RAISE_EXCEPTION(PyExc_TypeError, "Pocket cards must be 0-51 or 255.");
    }

    for (i = 0; i < n_board; i++)
    {
    	PyObject *item = PyList_GetItem(py_board, i);
    	if (!PyInt_Check(item))
	    	RAISE_EXCEPTION(PyExc_TypeError, "Board cards must be integers.");
    	board[i] = (int) PyInt_AsLong(item);
    	if ((board[i] < 0 || board[i] > 51) && board[i] != 255)
	    	RAISE_EXCEPTION(PyExc_TypeError, "Board cards must be 0-51 or 255.");
    }

   	if (is_omaha)
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

static PyMethodDef _rayeval_methods[] = {
	{"seed", (PyCFunction) _rayeval_seed, METH_VARARGS, "docstring"},
	{"load_handranks", (PyCFunction) _rayeval_load_handranks, METH_VARARGS, "docstring"},
	{"eval_mc", (PyCFunction) _rayeval_eval_mc, METH_VARARGS, "docstring"},
	{NULL, NULL, 0, NULL}
};

PyMODINIT_FUNC init_rayeval(void)
{
	init_random_int_52();
	Py_InitModule("_rayeval", _rayeval_methods);
}
