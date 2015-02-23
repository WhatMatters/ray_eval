/***
** Copyright (c) 2013-2015, Whatmatters Inc.
** Copyright (c) Ray Wooten
** Copyright (c) Kevin L. Suffecool
**
** It is free software, and may be redistributed under the terms specified in the
** GNU GENERAL PUBLIC LICENSE Version 2 or higher (see LICENSE.txt file).
** 
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
** INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
** DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
** SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
** WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
***/

#define MAX_PLAYERS 		10

#define READ_BLOCK_SIZE     2000000000

#define MAX(x, y)           ((x) > (y) ? (x) : (y))
#define MIN(x, y)           ((x) < (y) ? (x) : (y))

#define	STRAIGHT_FLUSH		1
#define	FOUR_OF_A_KIND		2
#define	FULL_HOUSE			3
#define	FLUSH				4
#define	STRAIGHT			5
#define	THREE_OF_A_KIND		6
#define	TWO_PAIR			7
#define	ONE_PAIR			8
#define	HIGH_CARD			9

#define CLUB				0x8000
#define DIAMOND 			0x4000
#define HEART   			0x2000
#define SPADE   			0x1000

#include <sys/types.h>

int load_file(char* dest, size_t size, size_t nitems, FILE* stream);
int cactus_findit(int key);
int cactus_to_ray(int holdrank);
const char *hand_rank_str(int handrank_num);
const char *get_hand_rank(int handrank);
void init_random_int_52();
void swap(int *x, int *y);
void random_sample_52_ross(int n, int k, int *out);
int *smart_load(const char *filename);
int smart_save(int *x, int size, const char *filename);
key_t generate_random_shm_key(void);
int *smart_load_to_shm(const char *filename, key_t key);
int *attach_hr(key_t key);
int *attach_shm(key_t key, int size);
int del_shm(key_t key);
