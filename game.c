#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <ctype.h>
#include "game.h"

static int total_moves = 0;

struct board *board_create( void )
{
  struct board *board = malloc( sizeof( struct board ) );
  if ( !board )
    exit_game("Unable to create the board.");

  int pieces[64] = { 0, 2, 3, 4, 5, 6, 4, 3, 2, 1, 1, 1, 1, 1, 1, 1, 1 };
  // each color keeps track of its pieces in a separate array
  for ( int i=1; i<=64; i++ ) { // initialize pieces for both sides
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
  free( board );
}

void help( void )
{
  puts("1) To move enter the starting and ending x position and y position ");
  puts("   of the moved piece in the form XYXY");
  puts("2) To castle kingside enter KK, queenside KQ");
}

struct player *player_create( int color )
{
  int x[]         = { 1, 2, 3, 4, 5, 6, 7, 8, 1, 2, 3, 4, 5, 6, 7, 8 };
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
  player->color = color;
  player->en_passant = 0;
  player->king_moved = 0;

  for (int i=0; i<16; i++ ) { // initialize pieces and positions.
    player->pieces->x[i] = x[i];
    player->pieces->y[i] = player->color==1 ? white_y[i] : black_y[i];
    player->pieces->active[i] = 1;
    player->pieces->type[i] = init_pieces[i];
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

      if ( (piece=get_piece(board->black[l])) ) {
        printf(P2_COLOR " %c ", piece );

      } else if ( (piece=get_piece(board->white[l])) ) {
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
    case 0:   return '\0';
    case 1:   return 'P';
    case 2:   return 'R';
    case 3:   return 'N';
    case 4:   return 'B';
    case 5:   return 'Q';
    case 6:   return 'K';
    default:  return '\0';
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

int turn( struct player *player )
{
  int move[4]; // 0: From X, 1: From Y, 2: To X, 3: To Y
  int temp_turn[3];

  total_moves++;
  print_board();

  if ( total_moves<3 )
    printf("Enter HELP for directions.\n");

  get_move:
    if ( in_check( player->color, player->pieces->x[12], player->pieces->y[12] ) )
      printf("You are in check.\n");

    printf("%s: ", player->color==WHITE ? "White" : "Black" );
    fflush(stdout);
    fflush(stdin);

    for ( int i=0; i<=4; i++ ) {
      move[i] = getchar();

      if ( i==1 && ( move[i]=='K' || move[i]=='Q' ) && !castle( player->color, ( move[i]=='K' ? KING : QUEEN ) ) )
          goto get_move; // player can't castle

      if ( i==2 && move[i]=='L' ) {
        print_board();
        help();
        goto get_move;
      }

      if ( move[i]>72 )  // convert lowercase to upper
        move[i] = toupper( move[i] );
      if ( move[i]>64 )
        move[i] -= 64;  // convert all letters to their row-integer equivalent
      else
        move[i] -= 48;  // convert ASCII numbers to their actual integer value
    }
    fflush(stdin);

    // are both moves referencing a location actually on the board?
    if ( !in_bounds( move[0], move[1] ) || !in_bounds( move[2], move[3] ) ) {
      move_error("Move request out of bounds.");
      goto get_move;
    }

    int from_location = get_location( move[0], move[1] ); // moving from
    int to_location   = get_location( move[2], move[3] ); // moving to

    // piece positions on the board
    int *player_pieces, *opponent_pieces;
    player_pieces   = player->color==1 ? board->white : board->black;
    opponent_pieces = player->color==1 ? board->black : board->white;

    if ( player_pieces[from_location]==0 ) { // is there a piece to move?
      move_error("Player has no piece to move at that position.");
      goto get_move;
    } else if ( player_pieces[to_location]!=0 ) { // is the target occupied by the same color?
      move_error("Player can't capture own pieces.");
      goto get_move;
    } else if ( !is_move_legal( player_pieces[from_location], move, player->color ) ) { // move legal?
      move_error("This is not a legal move for this piece.");
      goto get_move;
    }

    // save the state of the board.
    temp_turn[0] = player_pieces[to_location];
    temp_turn[1] = player_pieces[from_location];
    temp_turn[2] = opponent_pieces[to_location];

    // make the move.
    player_pieces[to_location]   = player_pieces[from_location];
    player_pieces[from_location] = 0;
    opponent_pieces[to_location] = 0;

    if ( in_check( player->color, player->pieces->x[12], player->pieces->y[12] ) ) { 
      // if the king is now in check, undo move and alert player.
      player_pieces[to_location]   = temp_turn[0];
      player_pieces[from_location] = temp_turn[1];
      opponent_pieces[to_location] = temp_turn[2];
      move_error("This move would put or leave your king in check.");
      goto get_move;
    }

    // move finalized
    struct player *opponent = player->color==1 ? black : white;

    for (int i=0; i<16; i++) { 
      // adjust the piece position in the player struct
      if ( player->pieces->x[i]==move[0] && player->pieces->y[i]==move[1] ) {
        player->pieces->x[i] = move[2];
        player->pieces->y[i] = move[3];

        // flag potential en passant rule on the next move clear last flag
        if ( player_pieces[to_location]==PAWN && abs(move[1]-move[3])==2 )
          player->en_passant = 1;
        else
          player->en_passant = 0;

        if ( player_pieces[to_location]==KING )
          player->king_moved = 1; // player can no longer castle

        if ( player_pieces[from_location]==ROOK && ( from_location==1 || from_location==57) )
          player->queen_rook_moved; // player can no longer castle
        else if ( player_pieces[from_location]==ROOK && ( from_location==8 || from_location==64) )
          player->king_rook_moved; // player can no longer castle

        // check if player is capturing with the en passant rule
        if ( player->pieces->type[i]==PAWN && opponent->en_passant && en_passant( player, move ) ) {
          // remove piece, award points
          opponent_pieces[get_location(move[2],move[1])] = 0;
          player->points += capture_piece( opponent, move[2], move[0] );
        }
      }
      // mark the opponent piece as inactive and award points to player
      player->points += capture_piece( opponent, move[1], move[2] );
    }

    // handle pawn promotion.
    if ( move[3]==1 && player_pieces[to_location]==1 )
      player_pieces[to_location] = pawn_promotion();

  return 1; 
}

int capture_piece( struct player *player, int x, int y )
{
  for (int i=0; i<16; i++) {
    if ( player->pieces->x[i]==x && player->pieces->y[i]==y ) {
      player->pieces->x[i] = 0;
      player->pieces->y[i] = 0;
      player->pieces->active[i] = 0;
      switch ( player->pieces->type[i] ) {
        case 1: return 1;  break;
        case 2: return 5;  break;
        case 3: return 3;  break;
        case 4: return 3;  break;
        case 5: return 9;  break;
        default:  break;
      }
    }
  }
  return 0;
}

int in_check( piece_color color, int x, int y )
{
  int enemy_piece, directions[4], bounds[4], blocked[4] = {0}, blocked_v[4] = {0}, move[2];
  
  int kingx = x;
  int kingy = y;

  for ( int i=1; i<10; i++ ) { // horizontal and vertical check
    directions[0] = get_location( kingx, kingy+i ); // up
    bounds[0]     = in_bounds(    kingx, kingy+i );
    directions[1] = get_location( kingx, kingy-i ); // down
    bounds[1]     = in_bounds(    kingx, kingy-i );
    directions[2] = get_location( kingx-i, kingy ); // left
    bounds[2]     = in_bounds(    kingx-i, kingy );
    directions[3] = get_location( kingx+i, kingy ); // right
    bounds[3]     = in_bounds(    kingx+i, kingy );
    for ( int j=0; j<4; j++ ) {
      if ( !blocked[j] ) {
        if ( bounds[j] && (color==WHITE ? board->white[directions[j]] : board->black[directions[j]])!=0 )
          blocked[j]++;
        else if ( bounds[j] ) {
          enemy_piece = (color==WHITE ? board->black[directions[j]] : board->white[directions[j]]);
          if ( enemy_piece==2 || enemy_piece==5 )
            return 1;
        }
      }
    }
  }

  for ( int i=1; i<10; i++ ) { // diagonal checks
    directions[0] = get_location( kingx-i, kingy+i ); // up-left
    bounds[0]     = in_bounds(    kingx-i, kingy+i );
    directions[1] = get_location( kingx+i, kingy+i ); // up-right
    bounds[1]     = in_bounds(    kingx+i, kingy+i );
    directions[2] = get_location( kingx-i, kingy-i ); // down-left
    bounds[2]     = in_bounds(    kingx-i, kingy-i );
    directions[3] = get_location( kingx+i, kingy-i ); // down-right
    bounds[3]     = in_bounds(    kingx+i, kingy-i );

    for ( int j=0; j<4; j++ ) {
      if ( !blocked_v[j] ) {
        if ( bounds[j] && (color==WHITE ? board->white[directions[j]] : board->black[directions[j]])!=0 )
          blocked_v[j]++;
        else if ( bounds[j] ) {
          enemy_piece = (color==WHITE ? board->black[directions[j]] : board->white[directions[j]]);
          if ( enemy_piece==4 || enemy_piece==5 )
            return 1;
        }
      }
    }
  }

  for ( int i=-1; i<2; i+=2 ) { // enemy knight check
    directions[0] = get_location( kingx+2, kingy+i ); // up
    bounds[0]     = in_bounds(    kingx+2, kingy+i );
    directions[1] = get_location( kingx-2, kingy+i ); // down
    bounds[1]     = in_bounds(    kingx-2, kingy+i );
    directions[2] = get_location( kingx+i, kingy+2 ); // left
    bounds[2]     = in_bounds(    kingx+i, kingy+2 );
    directions[3] = get_location( kingx+i, kingy-2 ); // right
    bounds[3]     = in_bounds(    kingx+i, kingy-2 );
    for ( int j=0; j<4; j++ ) {
      if ( bounds[j] && (color==WHITE ? board->black[directions[j]] : board->white[directions[j]])==3 )
        return 1;
    }
  }

  // attacking pawn check
  directions[0] = get_location( kingx-1, (color==WHITE ? (kingy+1) : (kingy-1)) );
  bounds[0]     = get_location( kingx-1, (color==WHITE ? (kingy+1) : (kingy-1)) );
  directions[1] = get_location( kingx+1, (color==WHITE ? (kingy+1) : (kingy-1)) );
  bounds[1]     = get_location( kingx+1, (color==WHITE ? (kingy+1) : (kingy-1)) );
  for ( int i=0; i<2; i++ )
    if ( bounds[i] && (color==WHITE ? board->black[directions[i]] : board->white[directions[i]])==1 )
      return 1;

  return 0;
}

int en_passant( struct player *player, int *move )
{
  int tx = move[2]; // moving to (target square) x-position
  int ty = move[3]; // moving to (target square) y-position
  int fx = move[0]; // moving from (origin square) x-position
  int fy = move[1]; // moving from (origin square) y-position
  int enemy_piece = get_location( tx, fy );

  int *opponent_pieces = player->color==WHITE ? board->black : board->white;

  if ( tx!=fx && opponent_pieces[enemy_piece]==PAWN && fy==(player->color==WHITE ? 5 : 4) )
    return enemy_piece; // return location of piece to be captured.
  else
    return 0; // not en passant
}

int is_move_legal( int type, int *move, int color ) {

  int tx = move[2]; // moving to (target square) x-position
  int ty = move[3]; // moving to (target square) y-position
  int fx = move[0]; // moving from (origin square) x-position
  int fy = move[1]; // moving from (origin square) y-position
  int target_square = get_location( tx, ty );

  int directions[4], bounds[4], blocked[4]={0}, blocked_h[4]={0};

  switch( type ) {
    case 1: // Pawn:

      for ( int i=-1, sq_location; i<2; i++ ) { // check the available moves along the x-axis in front of the pawn.
        // white pieces move up, black will move down.
        sq_location = color==1 ? get_location( fy+i, fy+1) : get_location( fx+i, fy-1 );

        if ( i==0 && board->black[sq_location]!=0 && board->white[sq_location]!=0 )
          blocked[0]++; // the pawn has a piece directly in front blocking a two square first move.

        if ( sq_location==target_square ) { // the requested move is in the pawn's range.

          if ( i!=0 && ( color==1 ? board->black[sq_location] : board->white[sq_location] )==0 ) 
            return 0; // attacking diagonally, there is no enemy occupying the square.

          return 1; // the move is good.
        }
      }

      // if the pawn hasn't moved yet and the move request is for advancing two spaces.
      if ( fy==(color==1 ? 2 : 7) && ( get_location( fx, (color==1 ? (fy+2) : (fy-2)) )==target_square ) )
        return ( !blocked[0] ? 1 : 0 ); // if nothing is blocking the pawn the move is legal.

      break;

    case 3: // Knight: 
      for ( int i=-1; i<2; i+=2 ) { // check the two attack squares in each direction.
        if ( get_location( fx+2, fy+i )==target_square )
          return 1;
        if ( get_location( fx-2, fy+i )==target_square )
          return 1;
        if ( get_location( fx+i, fy+2 )==target_square )
          return 1;
        if ( get_location( fx+i, fy-2 )==target_square )
          return 1;
      }
      return 0;

      break;

    case 6: // King: can only move one square
      for ( int i=-1; i<2; i++ ) {
        if ( get_location( fx+i, fy-1 )==target_square )
          return 1;
        if ( get_location( fx+i, fy )==target_square )
          return 1;
        if ( get_location( fx+i, fy+1 )==target_square )
          return 1;
        }
      break;

    case 5: // Queen: falls through Rook and Bishop range checks.

    case 4: // Bishop: diagonal check
      for ( int i=1; i<9; i++ ) {
        // check squares in each direction, determine if square is on the board
        directions[0] = get_location( fx-i, fy+i );
        bounds[0]     = in_bounds(    fx-i, fy+i );
        directions[1] = get_location( fx+i, fy+i );
        bounds[1]     = in_bounds(    fx+i, fy+i );
        directions[2] = get_location( fx-i, fy-i );
        bounds[2]     = in_bounds(    fx-i, fy-i );
        directions[3] = get_location( fx+i, fy-i );
        bounds[3]     = in_bounds(    fx+i, fy-i );

        for ( int j=0; j<4; j++ ) { // for each direction at a distance of i squares
          if ( bounds[j] && directions[j]==target_square )
            return ( !blocked[j] ? 1 : 0 ); // there are no pieces in the way and the move is legal.
          else if ( bounds[j] && ( board->white[directions[j]]!=0 || board->black[directions[j]]!=0 ) ) 
            blocked[j]++; // there is a piece blocking this direction.
        }
      }
      if ( type!=5 ) // queen falls through
        break;

    case 2: // Rook: horizontal and vertical check
      for ( int i=1; i<9; i++ ) {
        // check squares in each direction, determine if square is on the board.
        directions[0] = get_location( fx, fy+i );
        bounds[0]     = in_bounds(    fx, fy+i );
        directions[1] = get_location( fx, fy-i );
        bounds[1]     = in_bounds(    fx, fy-i );
        directions[2] = get_location( fx-i, fy );
        bounds[2]     = in_bounds(    fx-i, fy );
        directions[3] = get_location( fx+i, fy );
        bounds[3]     = in_bounds(    fx+i, fy );

        for ( int j=0; j<4; j++ ) { // for each direction at a distance of i squares
          if ( bounds[j] && directions[j]==target_square )
            return ( !blocked_h[j] ? 1 : 0 ); // there are no pieces in the way and the move is legal.
          else if ( bounds[j] && ( board->white[directions[j]]!=0 || board->black[directions[j]]!=0 ) )
            blocked_h[j]++; // there is a piece blocking this direction.
        }
      }
      break;

    default:
      break;
  }

  return 0;
}

int castle( int color, int side )
{
  return 1;
}

int pawn_promotion( void ) 
{
  char request;

  get_promotion:
    print_board();

    printf("Which piece should the pawn be promoted to? (Q,R,B,N): ");
    fflush(stdin);
    request = getchar();

    switch( request ) {
      case 'Q':   case 'q':   return 5;
      case 'R':   case 'r':   return 2;
      case 'B':   case 'b':   return 4;
      case 'N':   case 'n':   return 3;

      default:  goto get_promotion;
    }
  return 5;
}
