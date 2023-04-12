all: fuzzer

fuzzer: src/fuzzer.c
	gcc -o fuzzer src/fuzzer.c
