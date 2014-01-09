#-*- coding: utf-8 -*-

import rayeval

if __name__ == '__main__':
    rayeval.load_handranks_9('/usr/local/shared/rayeval_hand_ranks_9.dat')
    rayeval.load_handranks_7('/usr/local/shared/rayeval_hand_ranks_7.dat')

    print rayeval.made_hand_type("Js 7c 5s", "6d 2c 4c 2h")
    print rayeval.draw_type("Js 7c 5s", "6d 2c 4c 2h")
    print 'Js 7c 5s', rayeval.texture_changing_cards_count('Js 7c 5s')
    print 'Kd 7s 2c', rayeval.texture_changing_cards_count('Kd 7s 2c')
    print 'Kd 2s 2c', rayeval.texture_changing_cards_count('Kd 2s 2c')
    print 'Ks 7s 2s', rayeval.texture_changing_cards_count('Ks 7s 2s')