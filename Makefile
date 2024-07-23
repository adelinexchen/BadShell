all: shellper shell

shellper: shellper.c
	gcc -Wall -pedantic -std=gnu99 -g -c -o $@ $<

shell: shell.c shellper.c
	gcc -Wall -pedantic -std=gnu99 -g -o $@ $< shellper.c

clean:
	rm shellper && rm shell
