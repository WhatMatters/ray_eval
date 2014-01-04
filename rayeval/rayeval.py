# -*- coding: utf-8 -*-

"""
Shared memory routines will only work if your OS is properly configured,
e.g. in Mac OS X run the following prior to using shm functionality:

    sudo sysctl -w kern.sysv.shmmax=1598029824
    sudo sysctl -w kern.sysv.shmall=700000
"""

import _rayeval
import itertools
import joblib
import pkg_resources

__card_list = list(''.join(c) for c in itertools.product(
    '23456789TJQKA', 'cdhs'))

__hand_rank_str__ = [
    "n/a", 
    "high card",
    "one pair",
    "two pair",
    "three of a kind",
    "straight",
    "flush",
    "full house",
    "four of a kind",
    "straight flush"
]


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


def get_handranks_7_filename():
    """
    Returns 7-card handranks filename (with path) generated from the setup.py
    """
    return pkg_resources.resource_string(__name__, '__rayeval_ranks_path__.txt').split('\n')[0]


def get_handranks_9_filename():
    """
    Returns 9-card handranks filename (with path) generated from the setup.py
    """
    return pkg_resources.resource_string(__name__, '__rayeval_ranks_path__.txt').split('\n')[1]


def load_handranks_7(filename=None):
    """
    Load 7-card handranks from file into process memory

    filename    : 7-card hand ranks file
    """
    filename = get_handranks_7_filename() if filename is None else filename
    _rayeval.load_handranks_7(filename)


def load_handranks_9(filename=None):
    """
    Load 9-card handranks from file into process memory

    filename    : 9-card hand ranks file
    """
    filename = get_handranks_9_filename() if filename is None else filename
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


def load_handranks_7_to_shm(filename=None, path=None, ftok_id=0):
    """
    Load 7-card handranks from file to IPC shared memory

    filename    : 7-card hand ranks file
    path        : ftok path param for generating shm key
    ftok_id     : ftok id param for generating shm key
    """
    filename = get_handranks_7_filename() if filename is None else filename
    path = path if path is not None else filename
    _rayeval.load_handranks_7_to_shm(filename, path, ftok_id)


def load_handranks_9_to_shm(filename=None, path=None, ftok_id=0):
    """
    Load 9-card handranks from file to IPC shared memory

    filename    : 9-card hand ranks file
    path        : ftok path param to generate shm key
    ftok_id     : ftok id param to generate shm key
    """
    filename = get_handranks_9_filename() if filename is None else filename
    path = path if path is not None else filename
    _rayeval.load_handranks_9_to_shm(filename, path, ftok_id)


def attach_handranks_7(path=None, ftok_id=0):
    """
    Attach 7-card handranks shared memory segment

    path    : ftok path param to generate shm key
    ftok_id : ftok id param to generate shm key
    """
    path = get_handranks_7_filename() if path is None else path
    _rayeval.attach_handranks_7(path, ftok_id)


def attach_handranks_9(path=None, ftok_id=0):
    """
    Attach 9-card handranks shared memory segment

    path    : ftok path param to generate shm key
    ftok_id : ftok id param to generate shm key
    """
    path = get_handranks_9_filename() if path is None else path
    _rayeval.attach_handranks_9(path, ftok_id)


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


def del_handranks_shm_7(path=None, ftok_id=0):
    """
    Deletes 7-card handranks shared memory segment

    Note that only the super-user or a process with an effective uid equal
    to the shm_perm.cuid or shm_perm.uid values in the data structure
    associated with the queue can do this.

    path    : ftok path param to generate shm key
    ftok_id : ftok id param to generate shm key
    """
    path = get_handranks_7_filename() if path is None else path
    _rayeval.del_handranks_shm(path, ftok_id)


def del_handranks_shm_9(path=None, ftok_id=0):
    """
    Deletes 9-card handranks shared memory segment

    Note that only the super-user or a process with an effective uid equal
    to the shm_perm.cuid or shm_perm.uid values in the data structure
    associated with the queue can do this.

    path    : ftok path param to generate shm key
    ftok_id : ftok id param to generate shm key
    """
    path = get_handranks_9_filename() if path is None else path
    _rayeval.del_handranks_shm(path, ftok_id)


def del_handranks_shm(path, ftok_id=0):
    """
    Deletes handranks shared memory segment

    Note that only the super-user or a process with an effective uid equal
    to the shm_perm.cuid or shm_perm.uid values in the data structure
    associated with the queue can do this.

    path    : ftok path param to generate shm key
    ftok_id : ftok id param to generate shm key
    """
    _rayeval.del_handranks_shm(path, ftok_id)


def is_loaded_to_shm_7(path=None, ftok_id=0):
    """
    Returns True if 7-card hand rank file loaded to shm and False otherwise or on error

    path    : ftok path param to generate shm key
    ftok_id : ftok id param to generate shm key
    """
    path = get_handranks_7_filename() if path is None else path
    return _rayeval.is_loaded_to_shm(path, ftok_id)


def is_loaded_to_shm_9(path=None, ftok_id=0):
    """
    Returns True if 9-card hand rank file loaded to shm and False otherwise or on error

    path    : ftok path param to generate shm key
    ftok_id : ftok id param to generate shm key
    """
    path = get_handranks_9_filename() if path is None else path
    return _rayeval.is_loaded_to_shm(path, ftok_id)


def is_loaded_to_shm(path, ftok_id=0):
    """
    Returns True if hand rank file loaded to shm and False otherwise or on error

    path    : ftok path param to generate shm key
    ftok_id : ftok id param to generate shm key
    """
    return _rayeval.is_loaded_to_shm(path, ftok_id)


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
    if game not in ('holdem', 'omaha'):
        raise ValueError('Invalid game type.')
    return game


def eval_hand(game='holdem', board='', pocket=''):
    game = parse_game(game)
    i_board = parse_board(board)
    i_pocket = parse_pocket(pocket, game)
    return _rayeval.eval_hand(game, i_board, i_pocket)


def hand_rank_str(game='holdem', board='', pocket=''):
    return __hand_rank_str__[eval_hand(game=game, board=board, pocket=pocket) >> 12]


def hand_draw_outs(game='omaha', board='', pocket='', draw='straight'):
    i_board = parse_board(board)
    i_pocket = parse_pocket(pocket, game)

    outs = 0
    nut_outs = 0

    for c in __card_list:
        i_c = card_to_rank(c)
        if i_c in i_board or i_c in i_pocket:
            continue
        new_board = board + ' ' + c
        if hand_rank_str(game, new_board, pocket) is draw:
            outs += 1
            if is_first_nuts(new_board, pocket, draw):
                nut_outs += 1
    return outs, nut_outs


def hand_draw_type(board='', pocket=''):
    s_outs, s_nut_outs = hand_draw_outs('omaha', board, pocket, 'straight')
    f_outs, f_nut_outs = hand_draw_outs('omaha', board, pocket, 'flush')
    outs = s_outs + f_outs
    nut_outs = s_nut_outs + f_nut_outs
    if outs == 0:
        return 'ND'
    if outs < 8:
        return 'WD'
    if outs < 11:
        if nut_outs < 3:
            return 'WD'
        return 'GD'
    return 'SD'


def is_first_nuts(board='', pocket='', draw=''):
    if first_nuts_type(board) != draw:
        return False

    i_board = parse_board(board)
    i_pocket = parse_pocket(pocket, 'omaha')
    cards_by_suit = [[], [], [], []]
    board_cards = []
    pocket_cards = []

    for card in i_board:
        c, s = divmod(card, 4)
        cards_by_suit[s].append(c)
        board_cards.append(c)

    for card in i_pocket:
        c, s = divmod(card, 4)
        cards_by_suit[s].append(c)
        pocket_cards.append(c)

    if draw is 'flush':
        for cards in cards_by_suit:
            if len(cards) > 2:
                cards.sort()
                cards.reverse()
                if cards[0] == 12 and cards[1] == 11 and cards[2] == 10:
                    return True

    if draw is 'straight':
        board_cards.sort()
        board_cards.reverse()
        si = -1
        if board_cards[0] - board_cards[2] < 5:
            si = 0
        elif len(board_cards) is 4 and board_cards[1] - board_cards[3] < 5:
            si = 1
        elif len(board_cards) is 5 and board_cards[2] - board_cards[4] < 5:
            si = 2

        nut_card = board_cards[si] + (4 - board_cards[si] + board_cards[si + 2])

        if si > -1:
            for i in range(5):
                if nut_card - i not in board_cards and nut_card - i not in pocket_cards:
                    return False
            return True

    return False


def hand_draw_nut_outs(game='omaha', board='', pocket='', draw='straight'):
    i_board = parse_board(board)
    i_pocket = parse_pocket(pocket, game)

    outs = 0
    for c in __card_list:
        i_c = card_to_rank(c)
        if i_c in i_board or i_c in i_pocket:
            continue
        if hand_rank_str(game, board + ' ' + c, pocket) is draw:
            outs += 1
    return outs


def first_nuts_type(board=''):
    i_board = parse_board(board)

    suits = [0, 0, 0, 0]
    cards = []

    for i in i_board:
        c, s = divmod(i, 4)
        suits[s] += 1
        cards.append(c)

    cards.sort()
    cards.reverse()

    p_c = -1
    for c in cards:
        if c == p_c:
            return 'full house'
        p_c = c

    if suits[0] > 2 or suits[1] > 2 or suits[2] > 2 or suits[3] > 2:
        return 'flush'

    if (len(cards) == 3 and cards[0] - cards[2] < 5) or \
            (len(cards) == 4 and (cards[0] - cards[2] < 5 or cards[1] - cards[3] < 5)) or \
            (len(cards) == 5 and (cards[0] - cards[2] < 5 or cards[1] - cards[3] < 5 or cards[2] - cards[4] < 5)):
        return 'straight'

    return 'set'


def texture_change(board='', next_card=''):
    current_nuts = first_nuts_type(board)
    new_nuts = first_nuts_type(board + ' ' + next_card)

    if current_nuts == new_nuts:
        return 'blank'
    return new_nuts


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


def eval_turn_outs_vs_random_omaha(flopBoard, pocket, iterations):
    i_board = parse_board(flopBoard)
    i_pocket = parse_pocket(pocket, 'omaha')
    iterations = int(iterations)
    cppresult = _rayeval.eval_turn_outs_vs_random_omaha(i_board, i_pocket, iterations)
    result = {}
    result['flop_ev'] = cppresult[0]
    outs = {}
    result['outs'] = outs
    for i in xrange(52):
        if cppresult[i + 1] > -0.00001:
            outs[rank_to_card(i)] = cppresult[i + 1]
    return result


def find_first_nuts_holdem(flopBoard):
    i_board = parse_board(flopBoard)
    cppresult = _rayeval.find_first_nuts_holdem(i_board)
    return __card_list[cppresult[0]] + ' ' + __card_list[cppresult[1]]


def get_first_nuts_change_next_street(flopBoard):
    i_board = parse_board(flopBoard)
    nuts = _rayeval.find_first_nuts_holdem(i_board)

    result = {}
    for c in __card_list:
        i_c = card_to_rank(c)
        if i_c in i_board: continue

        turn_arr = i_board + [i_c]
        turnnuts = _rayeval.find_first_nuts_holdem(turn_arr)
        if turnnuts[0] != nuts[0] or turnnuts[1] != nuts[1]:
            s = __card_list[turnnuts[0]] + ' ' + __card_list[turnnuts[1]]
            result[c] = s

    return result