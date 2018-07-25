CFLAGS=-O3 -Wall -Werror -g

BINARYNAME=lz78

OBJFILES=main.o bitfile.o dictionary.o lz78.o

all: $(BINARYNAME)

$(BINARYNAME): $(OBJFILES)
	$(CC) -o $@ $^

main.o: lz78.h
bitio.o: bitfile.h
dictionary.o: dictionary.h
lz78.o: lz78.h bitfile.h dictionary.h

clean:
	rm -rf $(OBJFILES) $(BINARYNAME)
