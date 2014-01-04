#-*- coding: utf-8 -*-

import rayeval

if __name__ == '__main__':
    rayeval.load_handranks_9('/usr/local/shared/rayeval_hand_ranks_9.dat')
    rayeval.load_handranks_7('/usr/local/shared/rayeval_hand_ranks_7.dat')

    print rayeval.hand_draw_type("9c 8s 2d", "Ad Qd 2c 3s")