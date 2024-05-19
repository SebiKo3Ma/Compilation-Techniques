SOURCES = *.c

acc: $(SOURCES) Makefile
	gcc -Wall -o acc $(SOURCES)