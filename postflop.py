#-*- coding: utf-8 -*-

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

import rayeval

if __name__ == '__main__':
    rayeval.load_handranks_9('/usr/local/shared/rayeval_hand_ranks_9.dat')
    rayeval.load_handranks_7('/usr/local/shared/rayeval_hand_ranks_7.dat')

    rayeval.made_hand_type_fast("Js 7c 5s", "6d 2c 4c 2h")
    rayeval.draw_type("Js 7c 5s", "6d 2c 4c 2h")

    rayeval.made_hand_type_fast("As Tc 3s 7d", "6d 2c 4c 2h")
    rayeval.draw_type("As Tc 3s 7d", "6d 2c 4c 2h")
#    print rayeval.draw_type("Js 7c 5s", "6d 2c 4c 2h")
