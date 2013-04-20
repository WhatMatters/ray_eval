
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
