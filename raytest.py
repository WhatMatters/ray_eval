# -*- coding: utf-8 -*-

import rayeval
import time

if __name__ == '__main__':
    rayeval.load_handranks('HandRanks.dat')
    rayeval.load_handranks_9('hr9_cpp4.dat')

    game = 'omaha'
    # board = '* * * * *'
    # pocket = ['Kh 9h Ks 9s', '* * * *', '* * * *', '* * * *']
    board = '2d 2h 3c * *'
    pocket = ['Kh 9h * *', 'As * * *']
    iterations = 1e7
    n_jobs = 1

    t0 = time.time()
    ev = rayeval.eval_mc(game, board, pocket, iterations, n_jobs)
    elapsed = time.time() - t0
    print '[%s]' % 'naive omaha', pocket[:1], 'vs', pocket[1:], (
        ': EV = %.4f%% (%.2gM iterations).' % (100. * ev[0], iterations / 1e6))
    print 'Elapsed: %.2f seconds (%.2fM iterations / sec).\n' % (
        elapsed, iterations / elapsed / 1e6)

    t0 = time.time()
    ev = rayeval.eval_mc('omaha_9', board, pocket, iterations, n_jobs)
    elapsed = time.time() - t0
    print '[%s]' % 'omaha_9', pocket[:1], 'vs', pocket[1:], (
        ': EV = %.4f%% (%.2gM iterations).' % (100. * ev[0], iterations / 1e6))
    print 'Elapsed: %.2f seconds (%.2fM iterations / sec).\n' % (
        elapsed, iterations / elapsed / 1e6)

    import pokereval
    t0 = time.time()
    board_pe = ['__' if b == '*' else b for b in board.split(' ')]
    pocket_pe = [['__' if c == '*' else c for c in p.split(' ')] for p in pocket]
    res = pokereval.PokerEval().poker_eval(game='omaha', board=board_pe,
                                           pockets=pocket_pe, iterations=int(iterations))
    elapsed = time.time() - t0
    print '[%s]' % 'pokereval', pocket[:1], 'vs', pocket[1:], (
        ': EV = %.4f%% (%.2gM iterations).' % (res['eval'][0]['ev'] / 10., iterations / 1e6))
    print 'Elapsed: %.2f seconds (%.2fM iterations / sec).' % (
        elapsed, iterations / elapsed / 1e6)
