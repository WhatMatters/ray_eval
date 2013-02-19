# -*- coding: utf-8 -*-

import _rayeval
import itertools
import joblib

__card_list = list(''.join(c) for c in itertools.product(
    '23456789TJQKA', 'cdhs'))


def card_to_rank(card):
    "Convert a string representation of a card to 0:51+255 value."
    if card in ('*', '__', '_'):
        return 255
    else:
        return __card_list.index(card[0].upper() + card[1].lower())


def rank_to_card(rank):
    "Convert 0:51+255 card rank to a string value."
    return __card_list[rank]


def load_handranks(filename):
    _rayeval.load_handranks(filename)


def seed(n):
    """
    Set the random seed for sampling to a specified values.
    """
    _rayeval.seed(n)


def eval_mc(game='holdem', board='', pocket=['', ''],
            iterations=1e6, n_jobs=1):
    """
    Some docstring goes here.
    """

    is_iterable = lambda x: isinstance(x, list) or isinstance(x, tuple)
    split_string = lambda x: [] if not x.strip() else [
        c.strip() for c in x.split(' ')]
    if game not in ('holdem', 'omaha'):
        raise ValueError('Invalid game type.')
    if isinstance(board, basestring):
        board = split_string(board)
    if not is_iterable(board):
        raise TypeError('Board must be a list, a tuple or a string.')
    if isinstance(pocket, basestring):
        pocket = split_string(pocket)
    if not is_iterable(pocket):
        raise TypeError('Pocket must be a list or a tuple.')
    iterations = int(iterations)
    n_board = len(board)
    if n_board is 0:
        board = ['*'] * 5
    elif n_board < 3 or n_board > 5:
        raise ValueError('Invalid board size.')
    n_players = len(pocket)
    if n_players <= 1 or n_players > 10:
        raise ValueError('Invalid number of players.')
    i_board = [card_to_rank(c) for c in board]
    pocket_size = 2 if game == 'holdem' else 4
    i_pocket = []
    for p in pocket:
        if isinstance(p, basestring):
            p = split_string(p)
        if not is_iterable(p):
            raise TypeError('Each pocket must be a list, a tuple or a string.')
        if len(p) > pocket_size:
            raise ValueError('Invalid pocket size for selected game type.')
        i_pocket.extend([card_to_rank(c) for c in p])
        i_pocket.extend([255] * (pocket_size - len(p)))
    if not isinstance(n_jobs, int) or n_jobs <= 0:
        raise ValueError('Invalid number of jobs.')
    if n_jobs is 1:
        return _rayeval.eval_mc(game, i_board, i_pocket, iterations)
    else:
        result = joblib.Parallel(n_jobs=n_jobs)(joblib.delayed(
            _rayeval.eval_mc)(game, i_board, i_pocket, iterations / n_jobs)
            for i in xrange(n_jobs))
        return [sum(c) / float(n_jobs) for c in zip(*result)]
