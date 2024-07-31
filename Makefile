all: commands shellper shell

commands: commands.c
	gcc -Wall -pedantic -std=gnu99 -g -c -o $@ $<

shellper: shellper.c
	gcc -Wall -pedantic -std=gnu99 -g -c -o $@ $<

shell: shell.c shellper.c commands.c 
	gcc -Wall -pedantic -std=gnu99 -g -o $@ $< shellper.c commands.c

clean:
	rm commands && rm shellper && rm shell
