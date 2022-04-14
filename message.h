#ifndef _MESSAGEH_
#define _MESSAGEH_

/**
 * Decoupe du message 
 * Retourne 1 si il n'y a pas eu de problème
 * Retourne 0 si il y a eu un problème
 * ATTENTION:
 *   Les variables nTest, type et valeur doivent 
 * être allouées avant l'appel à decoupe
 */

int decoupe(char *message,      /* Source a découper */
	    char *nTest,        /* no de carte concernee */
	    char *type,         /* Type du message */
	    char *valeur        /* Valeur associée au message */
	    );

/** 
 * Construction du message
 * Retourne le message construit suivant le protocole
 * a partir des arguments
 * Le message est alloué dans la fonction
 */
char* message(char *nTest, char *type, char * valeur);

#endif // _MESSAGEH_
