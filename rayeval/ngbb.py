# -*- coding: utf-8 -*-

import rayeval
import time

if __name__ == '__main__':
    # rayeval.load_handranks('HandRanks.dat')
    rayeval.load_handranks_9('/usr/local/shared/rayeval_hand_ranks_9.dat')

    board = 'Ks Jd 9d'
    pocket = '9s Kd 4d 6c'
    iterations = 1e6

    t0 = time.time()
    ev = rayeval.eval_turn_outs_vs_random_omaha(board, pocket, iterations)
    elapsed = time.time() - t0

    print pocket, 'on board [%s]' % board, ev
    print 'Elapsed: %.2f seconds (%.2fM iterations / sec).\n' % (elapsed, iterations / elapsed / 1e6)