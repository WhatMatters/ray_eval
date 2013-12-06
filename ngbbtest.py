# -*- coding: utf-8 -*-

import rayeval
import time

if __name__ == '__main__':
    # rayeval.load_handranks('HandRanks.dat')
    rayeval.load_handranks_9('/usr/local/shared/rayeval_hand_ranks_9.dat')

    board = 'Ks Jd 9d * *'
    pockets = ['9s Kd 4d 6c']
    iterations = 1e6

    t0 = time.time()
    ngbb = rayeval.eval_turn_outs_vs_random_omaha(board, pockets[0], iterations)
    elapsed = time.time() - t0

    print '[%s]' % pockets, 'on board [%s]' % board
    print 'Elapsed: %.2f seconds (%.2fM iterations / sec).\n' % (elapsed, iterations / elapsed / 1e6)
    for item in sorted(ngbb.items(), key=lambda x : x[1], reverse=True):
        print item

    flop_ev = ngbb['flop_ev']
    outs = ngbb['outs']
    nuts = good = blank = bad = 0
    for k in outs:
        ev = outs[k]
        if ev >= 0.98:
            nuts = nuts + 1
        elif ev >= flop_ev + 0.1: 
            good = good + 1
        elif ev <= flop_ev - 0.1:
            bad = bad + 1
        else:
            blank = blank + 1

    print '[Nuts-Good-Blank-Bad] = [%d-%d-%d-%d]' % (nuts, good, blank, bad)

    print ''

    import pokereval
    pockets = pockets + ['* * * *']
    t0 = time.time()
    board_pe = ['__' if b == '*' else b for b in board.split(' ')]
    pocket_pe = [['__' if c == '*' else c for c in p.split(' ')] for p in pockets]
    res = pokereval.PokerEval().poker_eval(game='omaha', board=board_pe,
                                           pockets=pocket_pe, iterations=int(iterations))
    elapsed = time.time() - t0
    print '[%s]' % 'pokereval', pockets[:1], 'vs', pockets[1:], 'on board [%s]' % board, (
        ': EV = %.4f%% (%.2gM iterations).' % (res['eval'][0]['ev'] / 10., iterations / 1e6))
    print 'Elapsed: %.2f seconds (%.2fM iterations / sec).' % (
        elapsed, iterations / elapsed / 1e6)
