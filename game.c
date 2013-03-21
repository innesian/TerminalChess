#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include "game.h"

static int total_moves = 0;

struct board *board_create( void )
{
  struct board *board = malloc( sizeof( struct board ) );
  if ( !board )
    exit_game("Unable to create the board.");

  int pieces[64] = { 0, 2, 3, 4, 5, 6, 4, 3, 2, 1, 1, 1, 1, 1, 1, 1, 1 };

  for ( int i=1; i<=64; i++ ) {
    board->white[i] = 0;
    board->black[i] = 0;

    if ( i<17 )
      board->white[i] = pieces[i];  

    if ( i>=49 && i<=56 ) 
      board->black[i] = 1;

    else if ( i>56 )
      board->black[i] = pieces[i-56];
  }

  return board;
}

void board_destroy( void )
{
  free( theater );
}

struct player *player_create( int player_number )
{
  int x[] = { 1, 2, 3, 4, 5, 6, 7, 8, 1, 2, 3, 4, 5, 6, 7, 8 };
  int black_y[]   = { 7, 7, 7, 7, 7, 7, 7, 7, 8, 8, 8, 8, 8, 8, 8, 8 };
  int white_y[]   = { 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1 };
  piece_type init_pieces[] = { P, P, P, P, P, P, P, P, R, N, B, Q, K, B, N, R };

  struct player *player = malloc( sizeof( struct player ) );
  struct pieces *pieces = malloc( sizeof( struct pieces ) );

  if ( !player )
    exit_game("Unable to create player.");

  if ( !pieces )
    exit_game("Unable to set player pieces.");

  player->pieces = pieces;
  player->color = player_number;

  switch ( player_number ) {
    case 1:
      for (int i=0; i<=15; i++ ) {
        player->pieces->x[i] = x[i];
        player->pieces->y[i] = white_y[i];
        player->pieces->active[i] = 1;
        player->pieces->type[i] = init_pieces[i];
      }
      break;

    case 2:
      for (int i=0; i<=15; i++ ) {
        player->pieces->x[i] = x[i];
        player->pieces->y[i] = black_y[i];
        player->pieces->active[i] = 1;
        player->pieces->type[i] = init_pieces[i];
      }
      break;
  }

  return player;
}

void player_destroy( struct player *player )
{
  free( player->pieces );
  free( player );
}

void color_reset( void )
{
  printf(RESET_COLOR);
}

void exit_game( const char *exit_message )
{
  printf("GAME EXITED WITH ERROR: %s\n", exit_message );
  exit(1);
}

void print_board( void )
{
  char piece = '\0', letter_codes[] = { '\0', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h' };
  int background = 0;

  system(CLEAR_SCR);
  for ( int i=8; i>0; i-- ) {

    printf("\n%c ",letter_codes[i]);
    for ( int j=1, l=0; j<=8; j++ ) {

      l = get_location( j, i );

      if ( background%2 )
        printf(DARK_BACKGROUND);

      if ( (piece=get_piece(theater->black[l])) ) {
        printf(P2_COLOR " %c ", piece );

      } else if ( (piece=get_piece(theater->white[l])) ) {
        printf(P1_COLOR" %c ", piece );

      } else {
        printf("   ");
      }
      color_reset();
      background++;
    }
    background++;
  }
  printf("\n+  1  2  3  4  5  6  7  8\n");
}

char get_piece( int type )
{
  char piece;

  switch( type ) {
    case 0:
      return '\0';
    case 1:
      return 'P';
    case 2:
      return 'R';
    case 3:
      return 'N';
    case 4:
      return 'B';
    case 5:
      return 'Q';
    case 6:
      return 'K';
    default: return '\0';
  }
  return piece;
}

int get_location( int x, int y )
{
  if ( in_bounds( x, y ) ) {
    int position = ( ( x + ( ( y - 1 ) * ( MAX_X ) ) ) );
    return position;
  }
  return 0;
}

void play()
{
  static int move = 0, game_active = 1;

  while ( game_active ) {
    if ( move%2 )
      game_active = turn( black ); // black's move
    else
      game_active = turn( white ); // white's move
    move++;
  }

  // clean up
  player_destroy( white );
  player_destroy( black );
  board_destroy();
}

int in_bounds( int x, int y )
{
  return ( x<1 || x>8 || y<1 || y>8 ) ? 0 : 1;
}

void move_error( char *error )
{
  print_board();
  printf("%s\n", error);
}

int turn( struct player *p )
{
  int move[4]; // 0: From X, 1: From Y, 2: To X, 3: To Y
  int temp_turn[3];

  total_moves++;
  if ( total_moves<3 )
    printf("Enter HELP for directions.\n");

  print_board();

  get_move:
    if ( in_check( p ) )
      printf("You are in check.\n");

    printf("%s: ", p->color==1 ? "White" : "Black" );
    fflush(stdout);
    fflush(stdin);
    for ( int i=0; i<=4; i++ ) {
      move[i] = getchar();

      if( move[i]>72 )          // convert lowercase to upper
        move[i] = toupper( move[i] );

      if ( move[i]>64 )           // convert all letters to their row-integer equivalent
        move[i] -= 64;

      else                // convert ASCII numbers to their actual integer value
        move[i] -= 48;
    }

    // are both moves referencing a location actually on the board?
    if ( !in_bounds( move[0], move[1] ) || !in_bounds( move[2], move[3] ) ) {
      move_error("Move request out of bounds.");
      goto get_move;
    }

    // check if piece is on the from-square
    int from_location = get_location( move[0], move[1] );
    int to_location   = get_location( move[2], move[3] );

    if ( p->color==1 ) {
      if ( theater->white[from_location]==0 ) {
        // does the player have a piece to move?
        move_error("White has no piece to move at that position.");
        goto get_move;

      } else if ( theater->white[to_location]!=0 ) {
        // is the target square occupied by the same color?
        move_error("White can't capture white pieces.");
        goto get_move;
      } else if ( !is_move_legal( theater->white[from_location], move, WHITE ) ) {
        // is this a legal move, and no other pieces are in the way?
        move_error("This is not a legal move for this piece.");
        goto get_move;
      }

      // save the board, make the move, if it puts the player in check, don't move
      temp_turn[0] = theater->white[to_location];
      temp_turn[1] = theater->white[from_location];
      temp_turn[2] = theater->black[to_location];

      // make the move
      theater->white[to_location] = theater->white[from_location];
      theater->white[from_location] = 0;
      theater->black[to_location]   = 0;

      if ( in_check( p ) ) {
        // reverse the move
        theater->white[to_location] = temp_turn[0];
        theater->white[from_location] = temp_turn[1];
        theater->black[to_location] = temp_turn[2];
        move_error("This move would put your king in check.");
        goto get_move;
      }

      // handle pawn promotion
      if ( move[3]==8 && theater->white[to_location]==1 )
        theater->white[to_location] = pawn_promotion();
      
    } else if ( p->color==2 ) {
      if ( theater->black[from_location]==0 ) {
        move_error("Black has no piece to move at that position.");
        goto get_move;

      } else if ( theater->black[to_location]!=0 ) {
        move_error("Black can't capture white pieces.");
        goto get_move;

      } else if ( !is_move_legal( theater->black[from_location], move, BLACK ) ) {
        move_error("This is not a legal move for this piece.");
        goto get_move;
      }

      temp_turn[0] = theater->black[to_location];
      temp_turn[1] = theater->black[from_location];
      temp_turn[2] = theater->white[to_location];

      theater->black[to_location] = theater->black[from_location];
      theater->black[from_location] = 0;
      theater->white[to_location]   = 0;

      if ( in_check( p ) ) {
        theater->black[to_location] = temp_turn[0];
        theater->black[from_location] = temp_turn[1];
        theater->white[to_location] = temp_turn[2];
        move_error("This move would put or leave your king in check.");
        goto get_move;
      }

      if ( move[3]==1 && theater->black[to_location]==1 )
        theater->black[to_location] = pawn_promotion();
    }

  return 1; 
}

int in_check( struct player *player )
{
  int enemy_piece, directions[4], bounds[4], blocked[4] = {0}, blocked_h[4] = {0}, move[2];
  move[0] = player->pieces->x[12];
  move[1] = player->pieces->y[12];

  for ( int i=1; i<10; i++ ) { // horizontal and vertical check
    directions[0] = get_location( move[0], move[1]+i );
    bounds[0]     = in_bounds( move[0], move[1]+i );
    directions[1] = get_location( move[0], move[1]-i );
    bounds[1]     = in_bounds( move[0], move[1]-i );
    directions[2] = get_location( move[0]-i, move[1] );
    bounds[2]     = in_bounds( move[0]-i, move[1] );
    directions[3] = get_location( move[0]+i, move[1] );
    bounds[3]     = in_bounds( move[0]+i, move[1] );
    for ( int j=0; j<4; j++ ) {
      if ( !blocked[j] ) {
        if ( bounds[j] && (player->color==1 ? theater->white[directions[j]] : theater->black[directions[j]])!=0 )
          blocked[j]++;
        else if ( bounds[j] ) {
          enemy_piece = player->color==1 ? theater->black[directions[j]] : theater->white[directions[j]];
          if ( enemy_piece==2 || enemy_piece==5 )
            return 1;
        }
      }
    }
  }

  for ( int i=1; i<10; i++ ) { // diagonal checks
    directions[0] = get_location( move[0], move[1]+i );
    bounds[0]     = in_bounds( move[0], move[1]+i );
    directions[1] = get_location( move[0], move[1]-i );
    bounds[1]     = in_bounds( move[0], move[1]-i );
    directions[2] = get_location( move[0]-i, move[1] );
    bounds[2]     = in_bounds( move[0]-i, move[1] );
    directions[3] = get_location( move[0]+i, move[1] );
    bounds[3]     = in_bounds( move[0]+i, move[1] );
    for ( int j=0; j<4; j++ ) {
      if ( !blocked_h[j] ) {
        if ( bounds[j] && (player->color==1 ? theater->white[directions[j]] : theater->black[directions[j]])!=0 )
          blocked_h[j]++;
        else if ( bounds[j] ) {
          enemy_piece = player->color==1 ? theater->black[directions[j]] : theater->white[directions[j]];
          if ( enemy_piece==4 || enemy_piece==5 )
            return 1;
        }
      }
    }
  }

  for ( int i=-1; i<2; i+=2 ) { // enemy knight check
    directions[0] = get_location( move[0]+2, move[1]+i );
    bounds[0]     = in_bounds( move[0]+2, move[1]+i );
    directions[1] = get_location( move[0]-2, move[1]+i );
    bounds[1]     = in_bounds( move[0]-2, move[1]+i );
    directions[2] = get_location( move[0]+i, move[1]+2 );
    bounds[2]     = in_bounds( move[0]+i, move[1]+2 );
    directions[3] = get_location( move[0]+i, move[1]-2 );
    bounds[3]     = in_bounds( move[0]+i, move[1]-2 );
    for ( int j=0; j<4; j++ ) {
      if ( bounds[j] && (player->color==1 ? theater->black[directions[j]] : theater->white[directions[j]])==3 )
        return 1;
    }
  }

  // attacking pawn check
  directions[0] = get_location( move[0]-1, (player->color==1 ? (move[1]+1) : (move[1]-1)) );
  bounds[0]     = get_location( move[0]-1, (player->color==1 ? (move[1]+1) : (move[1]-1)) );
  directions[1] = get_location( move[0]+1, (player->color==1 ? (move[1]+1) : (move[1]-1)) );
  bounds[1]     = get_location( move[0]+1, (player->color==1 ? (move[1]+1) : (move[1]-1)) );
  for ( int i=0; i<2; i++ )
    if ( bounds[i] && (player->color==1 ? theater->black[directions[i]] : theater->white[directions[i]])==1 )
      return 1;

  return 0;
}

int is_move_legal( int type, int *move, int color ) {
  
  int to_location = get_location( move[2], move[3] );
  int directions[4], bounds[4], blocked[4] = {0}, blocked_h[4] = {0};

  switch( type ) {
    case 1: // Pawn:

      for ( int i=-1, sq_location; i<=2; i++ ) { // check the available moves along the x-axis in front of the pawn.

        // white pieces move up, black will move down.
        sq_location = color==1 ? get_location( move[0]+i, move[1]+1) : get_location( move[0]+i, move[1]-1 );

        if ( i==0 && theater->black[sq_location]==0 && theater->white[sq_location]==0 )
          blocked[0]++; // the pawn has a piece directly in front blocking a two square first move.

        if ( sq_location==to_location ) { // the requested move is in the pawn's range.

          if ( i!=0 && ( color==1 ? theater->black[sq_location] : theater->white[sq_location] )==0 ) 
            return 0; // attacking diagonally, there is no enemy occupying the square.

          return 1; // the move is good.
        }
      }

      // if the pawn hasn't moved yet and the move request is for advancing two spaces.
      if ( move[1]==(color==1 ? 2 : 7) && ( get_location( move[0], (color==1 ? (move[1]+2) : (move[1]-2)) )==to_location ) )
        return ( !blocked[0] ? 1 : 0 ); // if nothing is blocking the pawn the move is legal.

      break;

    case 3: // Knight: 
      for ( int i=-1; i<2; i+=2 ) { // check the two attack square in each direction.
        if ( get_location( move[0]+2, move[1]+i )==to_location )
          return 1;
        if ( get_location( move[0]-2, move[1]+i )==to_location )
          return 1;
        if ( get_location( move[0]+i, move[1]+2 )==to_location )
          return 1;
        if ( get_location( move[0]+i, move[1]-2 )==to_location )
          return 1;
      }
      return 0;

      break;

    case 6: // King: can only move one square
      for ( int i=-1; i<2; i++ ) {
        if ( get_location( move[0]+i, move[1]-1 )==to_location )
          return 1;
        if ( get_location( move[0]+i, move[1] )==to_location )
          return 1;
        if ( get_location( move[0]+i, move[1]+1 )==to_location )
          return 1;
        }
      break;

    case 5: // Queen: falls through Rook and Bishop range checks.

    case 4: // Bishop: diagonal check
      for ( int i=1; i<9; i++ ) {
        // check squares in each direction, determine if square is on the board
        directions[0] = get_location( move[0]-i, move[1]+i );
        bounds[0]     = in_bounds( move[0]-i, move[1]+i );
        directions[1] = get_location( move[0]+i, move[1]+i );
        bounds[1]     = in_bounds( move[0]+i, move[1]+i );
        directions[2] = get_location( move[0]-i, move[1]-i );
        bounds[2]     = in_bounds( move[0]-i, move[1]-i );
        directions[3] = get_location( move[0]+i, move[1]-i );
        bounds[3]     = in_bounds( move[0]+i, move[1]-i );

        for ( int j=0; j<4; j++ ) { // for each direction
          if ( bounds[j] && directions[j]==to_location )
            return ( !blocked[j] ? 1 : 0 ); // there are no pieces in the way and the move is legal.
          else if ( bounds[j] && ( theater->white[directions[j]]!=0 || theater->black[directions[j]]!=0 ) ) 
            blocked[j]++; // there is a piece blocking this direction.
        }
      }
      if ( type!=5 )
        break;

    case 2: // Rook: horizontal and vertical check
      for ( int i=1; i<9; i++ ) {
        // check squares in each direction, determine if square is on the board.
        directions[0] = get_location( move[0], move[1]+i );
        bounds[0]     = in_bounds( move[0], move[1]+i );
        directions[1] = get_location( move[0], move[1]-i );
        bounds[1]     = in_bounds( move[0], move[1]-i );
        directions[2] = get_location( move[0]-i, move[1] );
        bounds[2]     = in_bounds( move[0]-i, move[1] );
        directions[3] = get_location( move[0]+i, move[1] );
        bounds[3]     = in_bounds( move[0]+i, move[1] );

        for ( int j=0; j<4; j++ ) { // for each direction
          if ( bounds[j] && directions[j]==to_location )
            return ( !blocked_h[j] ? 1 : 0 ); // there are no pieces in the way and the move is legal.
          else if ( bounds[j] && ( theater->white[directions[j]]!=0 || theater->black[directions[j]]!=0 ) )
            blocked_h[j]++; // there is a piece blocking this direction.
        }
      }
      break;

    default:
      break;
  }

  return 0;
}

int castle( struct player *player )
{
  return 1;
}

int pawn_promotion( void ) 
{
  char request;

  get_promotion:
    print_board();
    fflush(stdin);
    fflush(stdout);
    printf("Which piece should the pawn be promoted to? (Q,R,B,N): ");

    request = getchar();
    switch( request ) {
      case 'Q':
      case 'q':
        return 5;
      case 'R':
      case 'r':
        return 2;
      case 'B':
      case 'b':
        return 4;
      case 'N':
      case 'n':
        return 3;
      default:
        goto get_promotion;
    }
  return 5;
}
