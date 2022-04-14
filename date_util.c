#include <stdio.h>
#include <time.h>

/*
Nous avons jugé utile de créer un programme pour gérer les dates puisque la méthode intervient dans 
plusieurs programmes et comporte plusieurs lignes. Cele-ci permet de transformer une date de la forme
yyyy:mm:dd en timestamp permettant de calculer le temps s'étant écouler entre deux dates. 

Auteurs: Sébastien BOIS & Maxime BOUET
Version: 14/04/2022
*/

unsigned long getTimestamp(char * date){
    int y, m, d;
    struct tm when = {0}; // Structure d'une date
    sscanf(date, "%d:%d:%d", &y, &m, &d);
    when.tm_year = y - 1900; // Instrctions trouvée sur la doc de la structure
    when.tm_mon = m - 1; // Instrctions trouvée sur la doc de la structure
    when.tm_mday = d;
    when.tm_isdst = -1; // Instrctions trouvée sur la doc de la structure
    time_t timee = mktime(&when); // On transforme la structure en variavle de temps
    unsigned long diff = (unsigned long) difftime(timee, 0); // On récupère le nombre de secondes écoulées depuis epoch
    return diff; // Différence entre la date dans date et la date depuis epoch
}