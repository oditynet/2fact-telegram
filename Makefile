pam:
	gcc $(CFLAGS) -Wunused -c -fPIC -DHAVE_SHADOW -O2 2fact.c
	gcc $(LDFLAGS) -o 2fact.so -s -lpam -lcrypt --shared 2fact.o
clean:
	rm -rf 2fact.o 2fact.so
