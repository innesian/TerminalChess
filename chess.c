#include <stdio.h>
#include "game.h"

int main(void) 
{
  board   = board_create();
  white   = player_create( WHITE );
  black   = player_create( BLACK );

  play();

  return 0;
}
