# -*- coding: utf-8 -*-

import rayeval

if __name__ == '__main__':
    rayeval.load_handranks_7('/usr/local/shared/rayeval_hand_ranks_7.dat')

    boards = ['Ks Jd 9d', '2c 2h 7d', 'As Ks 7s', '3h 3c 3d', 
        '2c 4h 7s', 'Ad Qh 8s',
        'Kd 7h 2h', '2c 2d 2h 2s', 'Ad Td Js Qs']

    for board in boards:
        first_nuts = rayeval.find_first_nuts_holdem(board)

        dynamica = rayeval.get_first_nuts_change_next_street(board)

        print "Board =", board.replace(' ', '')
        print "First Nuts =", first_nuts.replace(' ', '')
        print "Major nuts =", ''.join(dynamica.keys()), len(dynamica)
        print

