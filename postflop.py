#-*- coding: utf-8 -*-

import rayeval

if __name__ == '__main__':
    rayeval.load_handranks_9('/usr/local/shared/rayeval_hand_ranks_9.dat')
    rayeval.load_handranks_7('/usr/local/shared/rayeval_hand_ranks_7.dat')

    rayeval.made_hand_type_fast("Js 7c 5s", "6d 2c 4c 2h")
    rayeval.draw_type("Js 7c 5s", "6d 2c 4c 2h")

    rayeval.made_hand_type_fast("As Tc 3s 7d", "6d 2c 4c 2h")
    rayeval.draw_type("As Tc 3s 7d", "6d 2c 4c 2h")
#    print rayeval.draw_type("Js 7c 5s", "6d 2c 4c 2h")
