#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "lectureEcriture.h"
#include "message.h"
#include <string.h>

#define PREFIX "[Serveur d'Acquisition]"
#define PREFIX2 "[Serveur d'Acquisition Etranger]"
#define ERROR "[Erreur]"

/*
Cette version du serveur d'acquisition nous permet de prendre en charge deux types de serveur:
les serveurs d'acquisition normaux et les serveurs d'acquisition étranger.

Le serveur d'acquisition fait le lien entre le serveur inter-archives, le serveur de validation et
le terminal dans le cas où celui-ci en est doté d'un. Il permet le reroutage des messages entre les
différents processus du programme. 

Dans un premier temps, le serveur d'acquisition créé les processus des serveurs de validation et des
terminaux. Il créé également une paire de tuyaux pour le serveur de validation et une paire de tuyaux
supplémentaire pour le terminal dans le cas où il existe. Le serveur interarchives fournit également
une paire de tuyau au serveur d'acquisition lui permettant de communiquer avec.

Dans le cas d'un serveur avec terminal, le serveur d'acquisition récupère le test à validé depuis le terminal et
vérifie si les 4 premiers digits de son numéro correspondent à l'identifiant du serveur. Si c'est le cas, 
le message est transmis au serveur de validation, qui validera le message ou non, renverra le message au serveur
d'acquisition qui le renverra lui-même au terminal. Sinon, le message sera transmis au serveur interarchives 
afin qu'il le reroute vers le serveur correspondant aux 4 premiers digits. Le serveur d'acquisition
attend alors la réponse du serveur interarchives avant de le retransmettre au terminal.

Dans le cas où le serveur n'est pas doté d'un terminal, celui-ci attend un message de la part du
serveur interarchives avant de le transmettre à son serveur de validation et d'attendre sa réponse.
Une fois reçue, la réponse est reroutée vers le serveur inter-archives qui l'acheminera vers le 
second serveur d'acquisition.

Le programme prend en paramètre:
- Un identifiant de 4 chiffres
- Le nombre de terminaux
- Le file descriptor d'écriture
- Le file descriptor de lecture

Auteurs: Sébastien BOIS & Maxime BOUET
Version: 14/04/2022
*/

int main(int argc, char * argv[]){
    printf("%s Lancement du serveur d'acquisition...\n", PREFIX);

    if(argc != 5){
        printf("%s Tous les arguments n'ont pas été passés en paramètres du serveur d'acquisition.\n", ERROR);
        return 1;
    }

    char idSeq[5];
    strcpy(idSeq, argv[1]); // 4 premiers digits ID
    int termCount = atoi(argv[2]); // Nombre de terminaux
    int fd1 = atoi(argv[3]); // Ligne d'écriture
    int fd2 = atoi(argv[4]); // Ligne de lecture

    // On créé la paire de tuyaux du serveur de validation
    int validation1[2];
    int validation2[2];
    int pipe1IDValid = pipe(validation1);
    int pipe2IDValid = pipe(validation2);

    printf("%s id:%s term:%d\n", PREFIX, idSeq, termCount);

    // Vérification des erreurs sur les tuyaux de validation. Un ID <= -1 implique une erreur
    // sur la création des tuyaux
    if(pipe1IDValid <= -1 || pipe2IDValid <= -1){
        printf("%s Erreur lors de la création des pipes de validation.\n", ERROR);
        return 1;
    }

    // Création du processus de validation
    int pidValid = fork();

    // Vérification des erreurs sur la création du serveur de validation. Un ID <= -1
    // implique une erreur sur la création du processus.
    if(pidValid <= -1){
        printf("%s Erreur lors de la création du processus de validation.\n", ERROR);
        return 1;
    }

    // Processus serveur de validation
    if(pidValid == 0){
        char arg1[50], arg2[50];
        sprintf(arg1, "%d", validation1[1]); // Ligne d'écriture transmise au serveur de validation
        sprintf(arg2, "%d", validation2[0]); // Ligne de lecture transmise au serveur de validation
        // On exécute le programme de validation avec les paramètres définis précédemment
        execlp("./validation", "./validation", arg1, arg2, NULL);
    }

    // Dans le cas où le serveur d'acquisition est dôté d'un terminal
    if(termCount != 0){ 
        // Création de la paire de tuyaux du terminal
        int terminal1[2];
        int terminal2[2];
        int pipe1IDTerm = pipe(terminal1);
        int pipe2IDTerm = pipe(terminal2);

        // Vérification des erreurs sur les tuyaux de terminal. Un ID <= -1 implique une erreur
        // sur la création des tuyaux
        if(pipe1IDTerm <= -1 || pipe2IDTerm <= -1){
            printf("%s Erreur lors de la création des pipes de terminal.\n", ERROR);
            return 1;
        }

        // Création du processus terminal
        int pidTerm = fork();

        // Vérification des erreurs sur la création du terminal. Un ID <= -1
        // implique une erreur sur la création du processus.
        if(pidTerm <= -1){
            printf("%s Erreur lors de la création du processus terminal.\n", ERROR);
            return 1;
        }

        // Execution du processus terminal
        if(pidTerm == 0){
            close(terminal1[0]);
            char arg1[50], arg2[50];
            sprintf(arg1, "%d", terminal1[1]); // Transmission de la ligne d'écriture du terminal
            sprintf(arg2, "%d", terminal2[0]); // Transmission de la ligne de lecture du terminal
            // On exécute le programme terminal avec les paramètres définis ci-dessus
            execlp("./terminal", "./terminal", arg1, arg2, NULL);
        }

        // Processus parent avec terminal
        char * msg;
        //On lit tous les messages en provenance du terminal jusqu'à ce que nous recevions une
        //instruction d'arret
        while(strcmp((msg = litLigne(terminal1[0])), "On arrete tout\n") != 0){
            // On vérifie la conformité du message reçu
            if(msg != NULL){
                printf("%s Message reçu par le serveur: %s\n", PREFIX, msg);
                char nTest[50], type[50], value[50];
                // On récupère les informations contenues dans le message et on les stocke dans des variables
                int res = decoupe(msg, nTest, type, value);

                // Vérification de la découpe après réception du message. Si res == 0, cela signifie qu'une 
                // erreur a eu lieu lors de la transmission
                if(res == 0){
                    printf("%s Une erreur a eu lieu lors de la réception du message par le serveur d'acquisition.\n", ERROR);
                    return 1;
                }

                char prefix[5];
                strncpy(prefix, nTest, 4); // On récupère les 4 premiers digits du numéro de test

                // On vérifie si le préfix du test correspond à l'id du serveur
                if(strcmp(idSeq, prefix) == 0){
                    ecritLigne(validation2[1], msg); // Reroutage du message vers le serveur de validation
                    printf("%s Message envoyé au serveur de validation.\n", PREFIX);
                    // On lit la réponse du serveur de validation.
                    msg = litLigne(validation1[0]);
                    printf("%s Message reçu par le serveur: %s\n", PREFIX, msg);
                }else{ // Sinon on reroute vers le serveur interarchives
                    printf("%s Reroutage du message vers le serveur inter-archives.\n", PREFIX);
                    ecritLigne(fd1, msg);
                    // On lit la réponse en provenance du serveur inter archives 
                    msg = litLigne(fd2);
                    printf("%s Message reçu par le serveur: %s\n", PREFIX, msg);
                }
                res = decoupe(msg, nTest, type, value);

                // Vérification de la découpe pour éviter les erreurs de transmission
                if(res == 0){
                    printf("%s Une erreur a eu lieu lors de la réception du message par le serveur d'acquisition.\n", ERROR);
                    return 1;
                }

                ecritLigne(terminal2[1], msg);
                printf("%s Message envoyé au terminal.\n", PREFIX);
            }
        }
        // On éteint le serveur de validation 
        ecritLigne(validation2[1], "On arrete tout\n");
        // On éteint le serveur inter archives
        ecritLigne(fd1, "On arrete tout\n"); 
        printf("%s Extinction du serveur d'acquisition...\n", PREFIX);
    }else{ // Serveur sans terminal
        // Processus parent sans terminal
        char * msg;
        // On lit tous les messages en provenance du serveur inter archives jusqu'à recevoir
        // une instruction d'arret
        while (strcmp((msg = litLigne(fd2)), "On arrete tout\n") != 0)
        {
            // On vérifie la conformité du message
            if(msg != NULL){
                printf("%s Message reçu par le serveur: %s.\n", PREFIX2, msg);
                char nTest[50], type[50], value[50];
                // On récupère les informations contenues dans le message et on les stockes dans des variables
                int res = decoupe(msg, nTest, type, value);

                // Vérification de la découpe pour éviter les erreurs de transmission
                if(res == 0){
                    printf("%s Une erreur a eu lieu lors de la réception du message par le serveur d'acquisition.\n", ERROR);
                    return 1;
                }

                ecritLigne(validation2[1], msg); // Reroutage vers serveur de validation
                msg = litLigne(validation1[0]); // On lit la réponse en proveance du serveur de validation
                printf("%s Message reçu du serveur de validation: %s\n", PREFIX2, msg);
                res = decoupe(msg, nTest, type, value);

                // Vérification de la découpe pour éviter les erreurs de transmission
                if(res == 0){
                    printf("%s Une erreur a eu lieu lors de la réception du message par le serveur d'acquisition.\n", ERROR);
                    return 1;
                }

                ecritLigne(fd1, msg); // Reroutage vers serveur inter archives
                printf("%s Message rerouté vers le serveur inter archives.\n", PREFIX2);
            }
        }
        ecritLigne(validation2[1], "On arrete tout\n"); // On éteint le serveur de validation
        printf("%s Extinction du serveur d'acquisition...\n", PREFIX2);
    }


    return 0;
}