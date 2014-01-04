#-*- coding: utf-8 -*-

import rayeval

if __name__ == '__main__':
    rayeval.load_handranks_9('/usr/local/shared/rayeval_hand_ranks_9.dat')

    print rayeval.first_nuts_type("Ad 9d Kd 6s")
    print rayeval.texture_change("Ad 9d Kd 6s", "6d")