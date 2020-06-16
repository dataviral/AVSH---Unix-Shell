avsh: main funcs
	gcc -g main.o funcs.o -pthread -lreadline -o avsh
main: avsh.c avsh.h
	gcc -c -g avsh.c -o main.o
funcs: avsh_funcs.c avsh.h
	gcc -c -g avsh_funcs.c -o funcs.o
run: avsh
	gdb --args ./avsh
clean:
	rm *.o
