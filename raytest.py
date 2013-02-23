# -*- coding: utf-8 -*-

import rayeval
import time

if __name__ == '__main__':
    rayeval.load_handranks('HandRanks.dat')
    # rayeval.load_handranks_9('hr9.dat')

    game = 'omaha'
    board = '2h 2d 2s * *'
    pocket = ['Kh 9h Ks 9s', 'As Ac Ad Ah']
    iterations = 1e6
    n_jobs = 1

    t0 = time.time()
    ev = rayeval.eval_mc(game, board, pocket, iterations, n_jobs)
    elapsed = time.time() - t0
    print '[%s]' % game, pocket[:1], 'vs', pocket[1:], (
        ': EV = %.4f%% (%.2gM iterations).' % (100. * ev[0], iterations / 1e6))
    print 'Elapsed: %.2f seconds (%.2fM iterations / sec).' % (
        elapsed, iterations / elapsed / 1e6)

    t0 = time.time()
    ev = rayeval.eval_mc('omaha_9', board, pocket, iterations, n_jobs)
    elapsed = time.time() - t0
    print '[%s]' % game, pocket[:1], 'vs', pocket[1:], (
        ': EV = %.4f%% (%.2gM iterations).' % (100. * ev[0], iterations / 1e6))
    print 'Elapsed: %.2f seconds (%.2fM iterations / sec).' % (
        elapsed, iterations / elapsed / 1e6)

    import pokereval
    t0 = time.time()
    pokereval.PokerEval().poker_eval(game='omaha', board=['2h', '2d', '2s', '__', '__'],
        pockets=[['Kh', '9h', 'Ks', '9s'], ['__', '__', '__', '__'],
        ['__', '__', '__', '__'], ['__', '__', '__', '__']], iterations=int(1e6))
    elapsed = time.time() - t0
    print 'Elapsed: %.2f seconds (%.2fM iterations / sec).' % (
        elapsed, iterations / elapsed / 1e6)

    game = 'holdem'
    board = '* * * * *'
    pocket = ['Kh 9h', 'As Ac']
    iterations = 1e6
    n_jobs = 8

    t0 = time.time()
    ev = rayeval.eval_mc(game, board, pocket, iterations, n_jobs)
    elapsed = time.time() - t0
    print '[%s]' % game, pocket[:1], 'vs', pocket[1:], (
        ': EV = %.4f%% (%.2gM iterations).' % (100. * ev[0], iterations / 1e6))
    print 'Elapsed: %.2f seconds (%.2fM iterations / sec).' % (
        elapsed, iterations / elapsed / 1e6)
