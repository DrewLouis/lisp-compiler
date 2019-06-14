CFLAGS = -g -Wall

compiler : lisp-parse.c compiler.h
	$(CC) $(CFLAGS) -o lispv lisp-parse.c
