#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <Python.h>

#include "rayutils.h"
#include "raygen7.h"
#include "raygen9.h"

int *HR = 0, *HR9 = 0;

void extract_cards(uint64_t *deck, int card)
{
	*deck ^= (1LLU << card);
}

void get_cards(uint64_t deck, int *cards, int shift)
{
	int pos = 0;
	for (int i = 0; i < 52; i++)
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
				score = MAX(score, sf);
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
					score = MAX(score, HR[HR[HR[board_paths[nb] + 
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


static PyObject *_rayeval_load_handranks_7(PyObject *self, PyObject *args)
{
	char *filename;
  	if (!PyArg_ParseTuple(args, "s", &filename))
    	return NULL;
    if (!HR)
	    if (!(HR = smart_load(filename)))
	    	RAISE_EXCEPTION(PyExc_RuntimeError, "Failed to load hand ranks from file.");
	Py_RETURN_NONE;
}

static PyObject *_rayeval_load_handranks_9(PyObject *self, PyObject *args)
{
	char *filename;
  	if (!PyArg_ParseTuple(args, "s", &filename))
    	return NULL;
    if (!HR9)
	    if (!(HR9 = smart_load(filename)))
	    	RAISE_EXCEPTION(PyExc_RuntimeError, "Failed to load hand ranks [9] from file.");
	Py_RETURN_NONE;
}

static PyObject *_rayeval_generate_handranks_7(PyObject *self, PyObject *args)
{
	char *filename;
	int test;
  	if (!PyArg_ParseTuple(args, "si", &filename, &test))
    	return NULL;
	if (raygen7(filename, (test != 0)))
		RAISE_EXCEPTION(PyExc_RuntimeError, "Failed to generate hand ranks file.");
	Py_RETURN_NONE;
}

static PyObject *_rayeval_generate_handranks_9(PyObject *self, PyObject *args)
{
	char *filename, *filename7;
	int test;
  	if (!PyArg_ParseTuple(args, "ssi", &filename, &filename7, &test))
    	return NULL;
	if (raygen9(filename, filename7, (test != 0)))
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
				value = MAX(value, HR[HR[HR[board_paths[nb] + 
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
	{"generate_handranks_7", (PyCFunction) _rayeval_generate_handranks_7, METH_VARARGS, ""},
	{"generate_handranks_9", (PyCFunction) _rayeval_generate_handranks_9, METH_VARARGS, ""},
	{"load_handranks_7", (PyCFunction) _rayeval_load_handranks_7, METH_VARARGS, ""},
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
