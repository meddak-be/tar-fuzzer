all: fuzzer

fuzzer: fuzzer.c
	gcc -o fuzzer fuzzer.c

copy:
	rm archive2.tar && cp archive.tar archive2.tar

clean:
	rm test*