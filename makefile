all: fuzzer

fuzzer: fuzzer.c
	gcc -o fuzzer fuzzer.c