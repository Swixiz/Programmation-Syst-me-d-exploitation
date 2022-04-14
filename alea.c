#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "alea.h"

/**
 * Initialisation du générateur aléatoire
 */
void aleainit()
{
  srand( (unsigned)time( NULL ) + getpid() );
}

/**
 * Retourne un nombre aléatoire en min et max (bornes comprises)
 */
int alea(int min, int max)
{
  int lgr;

  lgr = max-min+1;
  return (int)((double)rand()/RAND_MAX*lgr+min);
}

