main: main.o evaler.o parser.o lexer.o
	gcc -o main main.o evaler.o parser.o lexer.o

main.o: main.c
	gcc -c main.c

evaler.o: evaler.c
	gcc -c evaler.c

parser.o: parser.c
	gcc -c parser.c

lexer.o: lexer.c
	gcc -c lexer.c

clean:
	rm -f *~ *.o *.a