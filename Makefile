all: fileCompressor.c fileObject.o
	gcc -g -o fileCompressor fileCompressor.c fileObject.o

fileObject.o: fileObject.c
	gcc -c fileObject.c

clean:
	rm fileCompressor; rm fileObject.o
