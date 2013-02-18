# -*- coding: utf-8 -*-

import rayeval
import time

if __name__ == '__main__':

    rayeval.load_handranks('HandRanks.dat')

    game = 'omaha'
    board = '2h 2d 2s * *'
    pocket = ['Kh 9h Ks 9s', '']
    iterations = 1e6
    n_jobs = 8

    t0 = time.time()
    ev = rayeval.eval_mc(game, board, pocket, iterations, n_jobs)
    elapsed = time.time() - t0
    print '[%s]' % game, pocket[:1], 'vs', pocket[1:], (
        ': EV = %.4f%% (%.2gM iterations).' % (100. * ev[0], iterations / 1e6))
    print 'Elapsed: %.2f seconds (%.2fM iterations / sec).' % (
        elapsed, iterations / elapsed / 1e6)
