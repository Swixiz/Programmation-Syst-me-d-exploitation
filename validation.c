#include <stdio.h>
#include "message.h"
#include <stdlib.h>
#include "lectureEcriture.h"
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include "date_util.h"

#define PREFIX "[Serveur de Validation]"
#define ERROR "[Erreur]"

/*
Cette version du serveur de validation nous permet de lire tous les résultats présents dans le
fichier res/res.txt et de convenir si oui ou non le test reçu par le serveur d'acquisition est
valide ou non (Temps entre la demande et la validation inférieur à 4 jours).

Une réponse est envoyée au serveur d'acquisition avec comme valeur 0 si le test est validé ou
1 si il est refusé.

Pour fonctionner, le programme prend deux file descriptors en paramètres correspondant aux
lignes de lecture et d'écriture attribuées par le serveur d'acquisition.

Auteurs: Sébastien BOIS & Maxime BOUET
Version: 14/04/2022
*/

int main(int argc, char * argv[]){
    printf("%s Lancement du serveur de validation...\n", PREFIX);

    // Vérification du nombre d'arguments passés en paramètres. On a normalement trois arguments:
    // Le nom du programme, la ligne d'écriture et la ligne de lecture
    if(argc != 3){
        printf("%s Nombre insuffisant d'arguments passés en paramètre.\n", ERROR);
        return 1;
    }

    int fd1 = atoi(argv[1]); // Ligne d'écriture
    int fd2 = atoi(argv[2]); // Ligne de lecture

    char * msg;
    // On lit en continue les messages reçus sur la ligne de lecture du serveur jusqu'à obtenir 
    // une instruction d'arret du programme.
    while(strcmp((msg = litLigne(fd2)), "On arrete tout\n") != 0){
        char nTest[50], type[50], value[50];
        // On récupère les informations du message et on les stockes dans les variables définies ci-dessus
        // Dans la variable value sera stockées le timestamp de la demande de validation du test
        int result = decoupe(msg, nTest, type, value);

        //Vérification que le message a bien été reçu sans erreur de transmission
        if(result == 0){
            printf("%s Erreur lors de la réception du message.\n", ERROR);
            return 1;
        }

        //On prépare le message de réponse 
        char * answer = (char *) malloc(strlen(msg) * sizeof(char));

        //On récupère le fichier de résultats et on l'ouvre en mode lecture
        FILE * resFile = fopen("../res/res.txt", "r");
        //On vérifie que le fichier existe et qu'il s'ouvre correctement 
        if(resFile == NULL){
            printf("%s Erreur lors de l'ouverture du fichier de resultat.\n", ERROR);
            return 1;
        }

        char line[50];
        unsigned long timestamp = 0;
        bool exists = false; // Variable permettant de vérifier que le test spécifié existe bien dans le fichier de résultats
        while (fgets(line, 50, resFile) != NULL)
        { // On parcout toutes les lignes du fichier de résultats
            char * prefix = strtok(line, " "); // On récupère le numéro de test de la ligne
            // Si le numéro du test correspond au prefix du message alors on actualise la varaible et
            // on récupère le timestamp.
            if(strcmp(prefix, nTest) == 0){ 
                exists = true;
                timestamp = getTimestamp(strtok(NULL, " "));
                break;
            }
        }
        fclose(resFile);

        // Si la ligne n'existe pas, on sort du programme
        if(!exists){
            printf("%s Le test PCR n'a pas été trouvé parmi les données du serveur de validation.\n", ERROR);
            return 1;
        }

        // On vérifie si la différence entre la date de la demande et la date des résultats est inférieure
        // a 4 jours. Si la date est inférieure, le test est validé, sinon il est refusé. En fonction,
        // on créé le message de réponse
        if(atoi(value) - timestamp >= 86600*4){ // Test trop vieux et donc refusé
             strcpy(answer, message(nTest, "réponse", "1"));
        }else{ // Test validé
            strcpy(answer, message(nTest, "réponse", "0"));
        }

        result = decoupe(msg, nTest, type, value);
        
        // Vérification de la découpe avant d'envoyer le message
        if(result == 0){
            printf("%s Une erreur a eu lieu lors de la génération du messgae\n", ERROR);
            return 1;
        }
        ecritLigne(fd1, answer); // On envoie le message de réponse au serveur d'acquisition
        printf("%s Message envoyé par le serveur.\n", PREFIX);
    }

    printf("%s Extinction du serveur de validation...\n", PREFIX);
    return 0;
}