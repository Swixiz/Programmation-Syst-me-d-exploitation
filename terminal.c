#include <stdio.h>
#include "message.h"
#include <stdlib.h>
#include "lectureEcriture.h"
#include <unistd.h>
#include <string.h>
#include <time.h>
#include "date_util.h"

#define PREFIX "[Terminal]"
#define ERROR "[Erreur]"

/*
Cette version du programme terminal récupère une liste de teste stockée dans le fichier res/demande.txt
Ceux-ci se présentent sous la forme suivante: 
numéro de test date

Le terminal lit chaque ligne de ce fichier avant de l'envoyer au serveur d'acquisition au travers des pipes
et file descriptors créés précedemment. Il attent ensuite la réponse de celui-ci pour pouvoir 
communiquer à la personne la validitée ou non de son test. 

Pour fonctionner, ce programme a besoin de deux file descriptors, un lui permettant de recevoir les
informations du serveur d'acquisition et l'autre le permettant de lui envoyer. 

Auteurs: Sébastien BOIS & Maxime BOUET
Version: 14/04/2022
*/

int main(int argc, char * argv[]){
    printf("%s Lancement du terminal...\n", PREFIX);
    
    // Vérification des arguments passés en paramètre. On a normalement trois arguments:
    // Le nom du programme, la ligne d'écriture et la ligne de lecture
    if(argc != 3){
        printf("%s Il manque des paramètres au terminal\n", ERROR);
        return 1;
    }
    // Ligne d'écriture récupérée sous forme d'entier
    // Un paramètre est un string, nous voulons donc le convertir en entier.
    // Nous utilisons donc la fonction atoi qui nous permet cette conversion.
    int fd1 = atoi(argv[1]);
    int fd2 = atoi(argv[2]); // Ligne de lecture
    
    // On récupère le fichier de demande et on l'ouvre en mode lecture.
    FILE * demandsFile = fopen("../res/demande.txt", "r");
    // Si le fichier est introuvable ou qu'une erreur survient lors de son ouverture, nous retournons une erreur
    if(demandsFile == NULL){
        printf("%s Erreur lors de l'ouverture du fichier demande.txt.\n", ERROR);
        return 1;
    }

    char line[50];
    // Lecture de chaque demande de traitement des test PCR stockée dans le fichier de demande
    while(fgets(line, sizeof(line), demandsFile) != NULL){
        char nTest[50];
        // On récupère le numéro de test en séparant la ligne au premier espace et on le stocke dans la variale nTest
        sprintf(nTest, "%s", strtok(line, " "));
        char type[10] = "demande"; // Le message créer est une demande au serveur de validation
        char valeur[50];
        // On récupère la date sous le format yyyy:mm:dd et on le transforme en timestamp grâce au programme
        // dateutil avant de le sauvegarder dans la variable valeur.
        sprintf(valeur, "%ld", getTimestamp(strtok(NULL, " ")));
        //On créé un message de la forme nTest|type|valeur
        char* msg = message(nTest, type, valeur);

        ecritLigne(fd1, msg); // On envoie le message créé précédemment au serveur d'acquisition
        printf("%s Message envoyé au serveur d'acquisition.\n", PREFIX);

        /* ---------------------------------------------------------------------------
            On attend la réponse du serveur d'acquisition à la demande envoyée ci-dessus
           ---------------------------------------------------------------------------*/

        char * validation = litLigne(fd2); // On lit la réponse en provenance du serveur d'acquisition
        char nTest2[50];
        // On découpe le message et on récupère ses informations
        int result = decoupe(validation, nTest2, type, valeur);
        printf("%s Message reçu par le terminal: %s\n", PREFIX, validation);

        // On vérifie si il y a eu une erreur dûe à la transmission du message dans sa découpe
        if(result == 0){
            printf("%s Une erreur a eu lieu lors de la génération du messgae\n", ERROR);
            return 1;
        }

        // On vérifie que le message envoyé et que le message reçu portent sur le même numéro de test
        if(strcmp(nTest, nTest2) == 0){
            // On vérifie que le message reçu est une réponse du serveur
            if(strcmp(type, "réponse") == 0){
                // On vérifie si le test a été validé
                if(strcmp(valeur, "0") == 0){ 
                    printf("%s Wow ! Wow ! Wow ! Le test a été validé !\n", PREFIX);
                }else{ // Le teste a été refusé par le serveur
                    printf("%s Aïe ! Coup dur pour guillaume... Ton test a été refusé.\n", PREFIX);
                }
            }else{
                printf("%s Le message reçu n'est pas une réponse du serveur.\n", ERROR);
            }
        }else{ // Message différent de celui attendu
            printf("%s Le résultat reçu n'est pas celui de ton test.\n", ERROR);
        }
        printf("---------------------------------------------\n");
    }
    // Instruction d'extinction du serveur d'acquisition
    ecritLigne(fd1, "On arrete tout\n");
    printf("%s Extinction du terminal.\n", PREFIX);
    return 0;   
}