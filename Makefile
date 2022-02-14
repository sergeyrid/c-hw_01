CFLAGS:=-Wall -Wextra -I include

main: obj/main.o obj/stego.o obj/bmp.o
	gcc obj/main.o obj/stego.o obj/bmp.o -o hw_01

obj:
	mkdir obj

obj/main.o: src/main.c include/stego.h include/bmp.h | obj
	gcc -c ${CFLAGS} src/main.c -o obj/main.o

obj/stego.o: src/stego.c include/stego.h include/bmp.h | obj
	gcc -c ${CFLAGS} src/stego.c -o obj/stego.o

obj/bmp.o: src/bmp.c include/bmp.h | obj
	gcc -c ${CFLAGS} src/bmp.c -o obj/bmp.o

.PHONY: clean
clean:
	rm -rf hw_01 obj
