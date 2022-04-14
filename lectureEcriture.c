#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "lectureEcriture.h"
#include <errno.h>

/* Define to 1 if your system has a GNU libc compatible `malloc' function, and
   to 0 otherwise. */
#define HAVE_MALLOC 1

/* Define to 1 if you have the <malloc.h> header file. */
/* #undef HAVE_MALLOC_H */

#if defined(HAVE_MALLOC_H)
#include <malloc.h>
#endif

char * litLigne(int fd)
{
  int i = 0, err = 0;
  char *c = malloc(TAILLEBUF+1);
  do {
    if( ((err = read(fd, &c[i],1)) <= 0) || (i == TAILLEBUF)){
      free(c);
      // read a positionn� l'erreur si il y en a une dans le read
      // Sinon, c'est que l'on d�passe TAILLEBUF
      if (i == TAILLEBUF)
	errno = EIO; // ligne trop grande
      return NULL;
    }
    i++;
  } while(c[i-1] != '\n');
  c[i] = '\0';
  return c;
}

int ecritLigne(int fd, char *c)
{
  int taille = strlen(c);
  if(write(fd, c, taille) == taille){
    return 1;
  }
    
  else {
    return 0;
  } // errno est positionn� par write.
}
