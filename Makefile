make: assembler.c lc3sim2.c
	gcc -std=c99 -o assembler assembler.c
	gcc -std=c99 -o lc3sim2 lc3sim2.c

run: assembler lc3sim2
	./assembler in.asm out.asm
	./lc3sim2 out.asm