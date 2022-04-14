#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

/* Define to 1 if your system has a GNU libc compatible `malloc' function, and
   to 0 otherwise. */
#define HAVE_MALLOC 1

/* Define to 1 if you have the <malloc.h> header file. */
/* #undef HAVE_MALLOC_H */

#if defined(HAVE_MALLOC_H)
#include <malloc.h>
#endif

/**
 * Decoupe du message 
 * Retourne 1 si il n'y a pas eu de problème
 * Retourne 0 si il y a eu un problème - errno est mis � EINVAL dans ce cas.
 * ATTENTION:
 *     Les variables nTest, type et valeur doivent 
 * être allouées avant l'appel à decoupe
 */
int decoupe(char *message,      /* Source a découper */
	    char *nTest,        /* no du test PCR concerne */
	    char *type,         /* Type du message */
	    char *valeur        /* Valeur associée au message */
	    )
{
  int nb=sscanf(message, "|%[^|]|%[^|]|%[^|]|\n", 
	 nTest, type, valeur);
  if (nb == 3)
    return 1;
  else  {
    errno = EINVAL;
    return 0;
  }
}

/** 
 * Construction du message
 * Retourne le message construit suivant le protocole
 * a partir des arguments
 * Le message est alloué dans la fonction
 */
char* message(char *nTest, char *type, char * valeur)
{
  int longueur = strlen(nTest) + strlen(type) + strlen(valeur) + 1 + 5 + 1;
  char *mess = calloc(longueur,sizeof(char));

  if (mess == NULL)
    return NULL;
  
  sprintf(mess, "|%s|%s|%s|\n", nTest, type, valeur);

  return mess;
}
