# CS 313 P3 Makefile
#   make           build test_bst and test_student
#   make check     build and run the autograder's tests on your bst.c
#   make test      build and run your three tests against your bst.c
#   make asan      run the autograder's tests with AddressSanitizer
#   make valgrind  run the leak scenario under valgrind (Linux; see SETUP.md)
#   make clean     remove built files

CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -g

.PHONY: all check test asan valgrind clean

all: test_bst test_student

test_bst: test_bst.c bst.c bst.h
	$(CC) $(CFLAGS) -o test_bst test_bst.c bst.c

test_student: test_student.c bst.c bst.h check.h
	$(CC) $(CFLAGS) -o test_student test_student.c bst.c

check: test_bst
	./test_bst

test: test_student
	./test_student

asan: test_bst.c bst.c bst.h
	$(CC) $(CFLAGS) -fsanitize=address,undefined -fno-sanitize-recover=undefined \
		-fno-omit-frame-pointer -o test_bst_asan test_bst.c bst.c
	./test_bst_asan

valgrind: test_bst
	valgrind --leak-check=full --error-exitcode=1 ./test_bst leak_scenario

clean:
	rm -rf test_bst test_bst_asan test_student *.dSYM
