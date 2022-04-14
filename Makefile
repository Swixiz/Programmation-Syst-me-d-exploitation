all: terminal acquisition validation inter

message.o: message.c message.h
	gcc -Wall -c message.c -o output/message.o

lectureEcriture.o: lectureEcriture.c lectureEcriture.h
	gcc -c -Wall lectureEcriture.c -o output/lectureEcriture.o

date_util.o: date_util.c date_util.h
	gcc -c -Wall date_util.c -o output/date_util.o

terminal: terminal.c output/lectureEcriture.o  output/message.o output/date_util.o
	gcc -Wall terminal.c output/lectureEcriture.o output/message.o output/date_util.o -o  output/terminal

validation: validation.c lectureEcriture.o message.o date_util.o
	gcc -Wall validation.c output/lectureEcriture.o output/message.o output/date_util.o -o  output/validation

acquisition: acquisition.c lectureEcriture.o message.o date_util.o
	gcc -Wall acquisition.c output/lectureEcriture.o output/message.o output/date_util.o -o  output/acquisition

inter: inter.c lectureEcriture.o message.o
	gcc -Wall inter.c output/lectureEcriture.o output/message.o -o output/inter

clean:	
	rm -f *.o