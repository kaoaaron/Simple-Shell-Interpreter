.phony all:
all: a1

a1: a1.c
	gcc a1.c -lreadline -lhistory -o a1

.PHONY clean:
clean:
	-rm -rf *.o *.exe
