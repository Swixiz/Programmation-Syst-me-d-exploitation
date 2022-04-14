#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

#include "lectureEcriture.h"

/**
 * Programme de test des fonctions lecture/ecriture
 */
int main(int argc, char **argv)
{
  int fd = open("./lectureEcriture.c", O_RDONLY);
  if (fd < 0) {
    perror("TestLectureEcriture - ./lectureEcriture.c");
    exit(0);
  }
  
  char *c = litLigne(fd);
  if (c == NULL) {
    perror("TestLectureEcriture - litLigne");
    exit(0);
  }

  int err = ecritLigne(W, c);
  if (err == 0) {
    perror("TestLectureEcriture - ecritLigne");
    exit(0);
  }
  
  close(fd);
  free(c);
  
  printf("On teste une erreur\n");
  c = litLigne(fd);
  if (c == NULL) {
    perror("TestLectureEcriture - litLigne");
    exit(0);
  } else {
    printf("Ca ne devrait pas arriver\n");
  }
  return 0;
}
