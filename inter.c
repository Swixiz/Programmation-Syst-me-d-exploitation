// Serveur inter archive
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "lectureEcriture.h"
#include "message.h"

#define PREFIX "[Serveur InterArchives]"
#define ERROR "[Erreur]"

/*
Cette version du programme permet la création de deux serveurs d'acquisition commuiquant entre eux
au travers du serveur inter-archives. Un des serveurs est dôté d'un terminal et d'un serveur de validation
tandis que le second ne possède qu'un unique serveur de validation.

Afin de communiquer entre eux, chaque processus est dôté d'une paire de tuyau et de file descriptors
passés en paramètres. 

Cette version du programme inter archives ne prends aucun paramètre.
Lorsque le serveur d'acquisition doté du terminal s'éteint, c'est-à-dire qu'il a fini de traiter toutes
ses données, le serveur inter-archives donne l'instruction d'extinction au second serveur avant 
de se terminer lui-même.

A la compilation, l'ensemble des executables sont disponibles dans le fichier output.

Auteurs: Sébastien BOIS & Maxime BOUET
Version: 14/04/2022
*/


int main(){
    printf("%s Lancement du serveur InterArchives.\n", PREFIX);

    int fdAcq01[2]; // Inter -> Acquisition 0
    int fdAcq02[2]; // Acquisition 0 -> Inter
    int fdAcq11[2]; // Inter -> Acquisition 1
    int fdAcq12[2]; // Acquisition 1 -> Inter

    int pipeIDAcq01 = pipe(fdAcq01);
    int pipeIDAcq02 = pipe(fdAcq02);
    int pipeIDAcq11 = pipe(fdAcq11);
    int pipeIDAcq12 = pipe(fdAcq12);

    // On vérifie que les pipes ont été créées sans problème ( ID <= -1 implique une erreur)
    if(pipeIDAcq01 <= -1 || pipeIDAcq02 <= -1 || pipeIDAcq11 <= -1 || pipeIDAcq12 <= -1){
        printf("%s Erreur lors de la création des pipes.\n", ERROR);
        return 1;
    }

    // Création du processus serveur acquisition avec 1 terminal (Serveur d'Acquisition)
    int pid0 = fork();

    // On vérifie que le processus s'est créé correctement (ID <= -1 implique une erreur)
    if(pid0 <= -1){
        printf("%s Erreur lors de la création du serveur d'acquisition 0.\n", ERROR);
        return 1;
    }

    if(pid0 == 0){
        char arg1[5], arg2[5], arg3[50], arg4[50];
        /* 
        Identifiant du seveur acquisition:
        Ce serveur ne vérifiera que les test dont le numéro commence par cet identifiant. (0123)
        */
        sprintf(arg1, "%s", "0123");
        sprintf(arg2, "%d", 1); // Le serveur est doté d'un terminal
        sprintf(arg3, "%d", fdAcq02[1]); // Transmission ligne d'écriture
        sprintf(arg4, "%d", fdAcq01[0]); // Transmission ligne de lecture
        // Execution du programme acquisition avec les paramètres définis ci-dessus
        execlp("./acquisition", "./acquisition", arg1, arg2, arg3, arg4, NULL);
    }
    // Création du processus serveur acquisition avec 0 terminal (Serveur d'Acquisition Etranger)
    int pid1 = fork();

    // On vérifie que le processus s'est créé correctement (ID <= -1 implique une erreur)
    if(pid1 <= -1){
        printf("%s Erreur lors de la création du serveur d'acquisition 1.\n", ERROR);
        return 1;
    }

    if(pid1 == 0){
        char arg1[5], arg2[5], arg3[50], arg4[50];
        /* 
        Identifiant du seveur acquisition étranger:
        Ce serveur ne vérifiera que les test dont le numéro commence par cet identifiant. (4567)
        */
        sprintf(arg1, "%s", "4567");
        sprintf(arg2, "%d", 0); // Le serveur n'est doté d'aucun terminal
        sprintf(arg3, "%d", fdAcq12[1]); // Transmission ligne d'écriture
        sprintf(arg4, "%d", fdAcq11[0]); // Transmission ligne de lecture
        // Execution du programme avec les paramètres définis ci-dessus
        execlp("./acquisition", "./acquisition", arg1, arg2, arg3, arg4, NULL);
    }

    // Processus serveur inter archives
    char * msg; 
    /* 
    On lit les messages en provenance du serveur d'acquisition dôté d'un terminal (Serveur 0)
    et tant qu'on ne réçoit de message forçant l'arret, nous continuons l'execution.
    */
    while (strcmp((msg = litLigne(fdAcq02[0])), "On arrete tout\n") != 0)
    {
        // Vérification de la conformité du message
        if(msg != NULL){
            printf("%s Message reçu par le serveur inter-archives %s\n", PREFIX, msg);
            char nTest[17], type[10], value[50];
            /* 
            On récupère les informations du message:
            - Numéro du test
            - Type de message: demande ou réponse
            - Valeur du test: 
                - demande: contient la date de la demande de validation enregistrée sur le terminal
                - réponse: 0 si le test a été validé par le serveur de validation et 1 si il a été refusé
            */
            int res = decoupe(msg, nTest, type, value);

            // Vérification que la découpe s'est effectuée sans problème
            // Si il y'a une erreur, res vaut 0
            if(res == 0){
                printf("%s Erreur lors de la découpe du message. Une erreur a du avoir lieu lors de sa transmission.\n", ERROR);
                return 1;
            }

            char prefix[5];
            strncpy(prefix, nTest, 4); // Récupération des 4 premiers digits du numéro de test PCR
            printf("%s prefix:%s\n", PREFIX, prefix);

            /*
            Vérifie si le prefix du numéro de test correspond à l'identifiant du serveur d'acquisition
            Etranger. Sinon il y a une erreur. Dans une future version, vérification de l'appartenance 
            à une table d'identifiant pour rerouter vers le serveur correspondant.
            */
            if(strcmp(prefix, "4567") == 0){
                ecritLigne(fdAcq11[1], msg); // Reroutage du message vers le serveur d'acquisition étranger
                printf("%s Message rerouté vers le serveur d'acquisition 1\n", PREFIX);
                msg = litLigne(fdAcq12[0]); // On lit la réponse du serveur d'acquisition étranger
                
                res = decoupe(msg, nTest, type, value); // On récupère les informations du message

                // Vérification de la découpe, une erreur peut survenir pendant la transmission du message
                if(res == 0){
                    printf("%s Erreur lors de la découpe du message. Une erreur a du avoir lieu lors de sa transmission.\n", ERROR);
                    return 1;
                }

                printf("%s Message rerouté vers le serveur d'acquisition 0\n", PREFIX);
                ecritLigne(fdAcq01[1], msg); // Redirection de la réponse vers le serveur d'acquisition 0 (serveur doté du terminal)
            }else{
                printf("%s Aucun serveur ne correspond à cet identifiant.\n", ERROR);
            }

        }else{
            printf("%s Erreur lors de la reception du message.\n", ERROR);
        }
    }
    // Instruction d'extinction du serveur d'acquisition étranger
    ecritLigne(fdAcq11[1], "On arrete tout\n"); 
    printf("%s Extinction du serveur inter archives...\n", PREFIX);

    return 0;
}