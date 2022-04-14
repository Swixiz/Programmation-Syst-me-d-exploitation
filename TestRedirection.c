#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


void usage(char * basename) {
    fprintf(stderr,
        "usage : %s [<programme 1> [<programme 2>]]\n",
        basename);
    exit(1);
}

int main(int argc, char *argv[]) {
    int pid;       /* permet d'identifier qui on est*/
    int fdpipe[2]; /* sera utilisé pour lier les processus */

    if (argc != 3) usage(argv[0]);

    /* on créé le pipe  qui sera utilisé pour relier 
       la sortie du premier processus 
       vers l'entrée du second 
    */
    if ( pipe(fdpipe) == -1 ) {
        perror("pipe");
        exit(-1);
    }

    switch(pid = fork()) {
        case -1:
            /* le fork a échoué */
            perror("fork");
            exit(-1);       
        case 0:
            /* code du fils */
            /* on fait en sorte que lorsque le processus 
               écrira sur l'entrée standard (1) 
               il le fera en fait dans le pipe (fdpipe[1])
            */
            dup2(fdpipe[1], 1);
            /* on ferme tout, même le pipe... 
               on n'en a plus besoin 
            */
            close(fdpipe[0]);
            close(fdpipe[1]);
            execlp(argv[1], argv[1],NULL);
            /* pas besoin de break,
               ce code n'existe déjà plus à l'exécution 
            */
        default :
            /* code du père */  
            /* on fait en sorte que lorsque le processus 
               lira sur la sortie standard (0) 
               il le fera en fait dans le pipe (fdpipe[0])
            */
            dup2(fdpipe[0], 0);
            close(fdpipe[0]);
            close(fdpipe[1]);
            execlp(argv[2],argv[2], NULL);
    }
   /*
   cette portion de code ne sera jamais exécutée, 
   puisque les processus ont déjà
   été remplacé. On met néanmoins un 
   return sinon le compilateur proteste.
   */
   return 0;
}
