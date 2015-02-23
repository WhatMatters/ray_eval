# -*- coding: utf-8 -*-
###
## Copyright (c) 2013-2015, Whatmatters Inc.
##
## It is free software, and may be redistributed under the terms specified in the
## GNU GENERAL PUBLIC LICENSE Version 2 or higher (see LICENSE.txt file).
## 
## THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
## INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
## DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
## SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
## SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
## WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
## OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
###

import csv

import itertools
import rayeval


def sort_flop(flop):
    a = flop[0][1]
    b = flop[1][1]
    c = flop[2][1]
    if a == b and b == c:
        return 1
    if a == b or b == c or a == c:
        return 2
    return 3


if __name__ == '__main__':
    card_list = list(''.join(c) for c in itertools.product('23456789TJQKA', 'cdhs'))

    all_flops = itertools.combinations(card_list, 3)

    all_flops = sorted(all_flops, key=sort_flop)

    outs = csv.writer(open("flop_dynamica.csv", "wb"))

    cnt = 0
    for board_cards in all_flops:

        board = ' '.join(board_cards)

        first_nuts = rayeval.find_first_nuts_holdem(board)

        dynamica = rayeval.get_first_nuts_change_next_street(board)

        outs.writerow([
            board.replace(' ', ''),
            first_nuts.replace(' ', ''),
            len(dynamica),
            ''.join(dynamica.keys())
        ])

        cnt += 1
        if cnt % 1000 == 0:
            print cnt
