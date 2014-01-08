# -*- coding: utf-8 -*-
import csv

import itertools
import rayeval


def sort_flop(flop):
    a = flop[0][1]
    b = flop[1][1]
    c = flop[2][1]
    if a == b and b == c:
        return 1
    if a == b or b == c or a == c:
        return 2
    return 3


if __name__ == '__main__':
    card_list = list(''.join(c) for c in itertools.product('23456789TJQKA', 'cdhs'))

    all_flops = itertools.combinations(card_list, 3)

    all_flops = sorted(all_flops, key=sort_flop)

    outs = csv.writer(open("flop_dynamica.csv", "wb"))

    cnt = 0
    for board_cards in all_flops:

        board = ' '.join(board_cards)

        first_nuts = rayeval.find_first_nuts_holdem(board)

        dynamica = rayeval.get_first_nuts_change_next_street(board)

        outs.writerow([
            board.replace(' ', ''),
            first_nuts.replace(' ', ''),
            len(dynamica),
            ''.join(dynamica.keys())
        ])

        cnt += 1
        if cnt % 1000 == 0:
            print cnt
