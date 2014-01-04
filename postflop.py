#-*- coding: utf-8 -*-

import rayeval

if __name__ == '__main__':
    rayeval.load_handranks_9('/usr/local/shared/rayeval_hand_ranks_9.dat')

    print rayeval.hand_rank_str('omaha', '9h 6d 2c 5d', 'Ad Ac 7s 8s')
    print rayeval.hand_draw_outs('omaha', '9h 6d 2c', 'Ad Ac 7s 8s', 'straight')
    print rayeval.hand_draw_outs('omaha', '9h 6d 2c', 'Ad Ac 7s 8s', 'flush')