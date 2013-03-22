#ifndef GAME_H
#define GAME_H

#define P1_COLOR "\x1B[33;1m"
#define P2_COLOR "\x1B[34;1m"
#define DARK_BACKGROUND "\x1b[47m"
#define RESET_COLOR "\x1b[0m"

#define MAX_X 8

#if defined(_WIN32) || defined(WIN32)
  #define CLEAR_SCR "cls"
#else
  #define CLEAR_SCR "clear"
#endif

#define PAWN    1
#define ROOK    2
#define KNIGHT  3
#define BISHOP  4
#define QUEEN   5
#define KING    6

typedef enum { P=1, R=2, N=3, B=4, Q=5, K=6 } piece_type;
typedef enum { WHITE=1, BLACK=2 } piece_color;

struct board {
  int black[64];
  int white[64];
};

struct board *board;

struct pieces {
  piece_type type[16];
  int x[16]; // horizontal piece position
  int y[16]; // vertical piece position
  int active[16];
};

struct player {
  piece_color color;
  int points;
  // x value of last move if it was made by a pawn and moved 2 spaces.
  int en_passant;
  int king_moved;
  int queen_rook_moved;
  int king_rook_moved;
  struct pieces *pieces;
};

struct player *white;
struct player *black;

void help( void );
int capture_piece( struct player *, int, int );
int en_passant( struct player *, int * );
int castle( struct player * );
int pawn_promotion( void );
int in_check( struct player * );
char get_piece( int );
struct board *board_create( void );
void board_destroy( void );
struct player *player_create( int );
void player_destroy( struct player * );
void print_board( void );
void color_reset( void );
void exit_game( const char * );
int get_location( int, int );
void play();
int turn( struct player * );
void move_error( char * );
int is_move_legal( int, int *, int );
int in_bounds( int, int );
#endif
