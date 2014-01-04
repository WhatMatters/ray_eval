#-*- coding: utf-8 -*-

import rayeval

if __name__ == '__main__':
    rayeval.load_handranks_9('/usr/local/shared/rayeval_hand_ranks_9.dat')
    rayeval.load_handranks_7('/usr/local/shared/rayeval_hand_ranks_7.dat')

    print rayeval.made_hand_type("9c 8s 2d", "Jc Qd 3c 9s")
    print rayeval.draw_type("9c 8d 2d", "Jd Qd 3c 9s")