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


def is_iterable(x):
    return isinstance(x, list) or isinstance(x, tuple)


def split_string(x):
    return [] if not x.strip() else [c.strip() for c in x.split(' ')]


def load_handranks_7(filename):
    """
    Load 7-card handranks

    filename    : 7-card hand ranks file
    """
    _rayeval.load_handranks_7(filename)


def load_handranks_9(filename):
    """
    Load 9-card handranks

    filename    : 9-card hand ranks file
    """
    _rayeval.load_handranks_9(filename)


def generate_handranks_7(filename, test=True):
    """
    Generate 7-card handranks

    filename    : 7-card hand ranks file
    test        : run the verification test
    """
    _rayeval.generate_handranks_7(filename, test)


def generate_handranks_9(filename, filename7='', test=True):
    """
    Generate 9-card handranks

    filename    : 9-card hand ranks file
    filename7   : 7-card hand ranks file
    test        : run the verification test
    """
    _rayeval.generate_handranks_9(filename, filename7, test)


def load_handranks_7_to_shm(filename, path=None, id=0):
    """
    Load 7-card handranks to IPC shared memory
        
    filename    : 7-card hand ranks file
    path        : ftok path param for generating shm key
    id          : ftok id param for generating shm key
    """
    path = path if path is not None else filename
    _rayeval.load_handranks_7_to_shm(filename, path, id)

                                  
def load_handranks_9_to_shm(filename, path=None, id=0):
    """
    Load 9-card handranks to IPC shared memory

    filename    : 9-card hand ranks file
    path        : ftok path param to generate shm key
    id          : ftok id param to generate shm key
    """
    path = path if path is not None else filename
    _rayeval.load_handranks_9_to_shm(filename, path, id)


def attach_handranks_7(path, id=0):
    """
    Attach 7-card handranks shared memory segment

    path    : ftok path param to generate shm key
    id      : ftok id param to generate shm key
    """
    _rayeval.attach_handranks_7(path, id)


def attach_handranks_9(path, id=0):
    """
    Attach 9-card handranks shared memory segment
                                      
    path    : ftok path param to generate shm key
    id      : ftok id param to generate shm key
    """
    _rayeval.attach_handranks_9(path, id)


def detach_handranks_7():
    """
    Detach 7-card handranks shared memory segment
    """
    _rayeval.detach_handranks_7()
                                  

def detach_handranks_9():
    """
    Detach 9-card handranks shared memory segment
    """
    _rayeval.detach_handranks_9()


def del_handranks_shm(path, id=0):
    """
    Deletes handranks shared memory segment
        
    Note that only the super-user or a process with an effective uid equal 
    to the shm_perm.cuid or shm_perm.uid values in the data structure 
    associated with the queue can do this.

    path    : ftok path param to generate shm key
    id      : ftok id param to generate shm key
    """
    _rayeval.del_handranks_shm(path, id)


def seed(n):
    """
    Set the random seed for sampling to a specified values.
    """
    _rayeval.seed(n)


def parse_board(board):
    if isinstance(board, basestring):
        board = split_string(board)
    if not is_iterable(board):
        raise TypeError('Board must be a list, a tuple or a string.')
    n_board = len(board)
    if n_board is 0:
        board = ['*'] * 5
    elif n_board < 3 or n_board > 5:
        raise ValueError('Invalid board size.')
    return [card_to_rank(c) for c in board]


def parse_pocket(pocket, game):
    pocket_size = 2 if game == 'holdem' else 4
    if isinstance(pocket, basestring):
        pocket = split_string(pocket)
    if not is_iterable(pocket):
        raise TypeError('Pocket must be a list, a tuple or a string.')
    if len(pocket) > pocket_size:
        raise ValueError('Invalid pocket size for selected game type.')
    return [card_to_rank(c) for c in pocket] + [255] * (pocket_size - len(pocket))


def parse_pockets(pockets, game):
    if isinstance(pockets, basestring):
        pockets = split_string(pockets)
    if not is_iterable(pockets):
        raise TypeError('Pockets must be a list or a tuple.')
    n_players = len(pockets)
    if n_players <= 1 or n_players > 10:
        raise ValueError('Invalid number of players.')
    i_pockets = []
    map(i_pockets.extend, map(lambda p: parse_pocket(p, game), pockets))
    return i_pockets


def parse_game(game):
    if game not in ('holdem', 'omaha', 'omaha_9'):
        raise ValueError('Invalid game type.')
    return game


def eval_hand(game='holdem', board='', pocket=''):
    game = parse_game(game)
    i_board = parse_board(board)
    i_pocket = parse_pocket(pocket, game)
    return _rayeval.eval_hand(game, i_board, i_pocket)


def which_hand(cards, value):
    import itertools
    for h in itertools.combinations(cards, 5):
        if eval_hand('holdem', h[:3], h[3:]) == value:
            return h


def eval_mc(game='holdem', board='', pockets=['', ''],
            iterations=1e6, n_jobs=1):
    game = parse_game(game)
    i_board = parse_board(board)
    i_pockets = parse_pockets(pockets, game)
    iterations = int(iterations)
    if not isinstance(n_jobs, int) or n_jobs <= 0:
        raise ValueError('Invalid number of jobs.')
    if n_jobs is 1:
        return _rayeval.eval_mc(game, i_board, i_pockets, iterations)
    else:
        result = joblib.Parallel(n_jobs=n_jobs)(joblib.delayed(
            _rayeval.eval_mc)(game, i_board, i_pockets, iterations / n_jobs)
            for i in xrange(n_jobs))
        return [sum(c) / float(n_jobs) for c in zip(*result)]



# def eval_mc(game='holdem', board='', pocket=['', ''],
#             iterations=1e6, n_jobs=1):
#     """
#     Some docstring goes here.
#     """

#     if game not in ('holdem', 'omaha', 'omaha_9'):
#         raise ValueError('Invalid game type.')
#     if isinstance(board, basestring):
#         board = split_string(board)
#     if not is_iterable(board):
#         raise TypeError('Board must be a list, a tuple or a string.')
#     if isinstance(pocket, basestring):
#         pocket = split_string(pocket)
#     if not is_iterable(pocket):
#         raise TypeError('Pocket must be a list or a tuple.')
#     iterations = int(iterations)
#     n_board = len(board)
#     if n_board is 0:
#         board = ['*'] * 5
#     elif n_board < 3 or n_board > 5:
#         raise ValueError('Invalid board size.')
#     n_players = len(pocket)
#     if n_players <= 1 or n_players > 10:
#         raise ValueError('Invalid number of players.')
#     i_board = [card_to_rank(c) for c in board]
#     pocket_size = 2 if game == 'holdem' else 4
#     i_pocket = []
#     for p in pocket:
#         if isinstance(p, basestring):
#             p = split_string(p)
#         if not is_iterable(p):
#             raise TypeError('Each pocket must be a list, a tuple or a string.')
#         if len(p) > pocket_size:
#             raise ValueError('Invalid pocket size for selected game type.')
#         i_pocket.extend([card_to_rank(c) for c in p])
#         i_pocket.extend([255] * (pocket_size - len(p)))
#     if not isinstance(n_jobs, int) or n_jobs <= 0:
#         raise ValueError('Invalid number of jobs.')
#     if n_jobs is 1:
#         return _rayeval.eval_mc(game, i_board, i_pocket, iterations)
#     else:
#         result = joblib.Parallel(n_jobs=n_jobs)(joblib.delayed(
#             _rayeval.eval_mc)(game, i_board, i_pocket, iterations / n_jobs)
#             for i in xrange(n_jobs))
#         return [sum(c) / float(n_jobs) for c in zip(*result)]
