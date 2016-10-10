OBJ = Main.o shell.o network.o
PA1 : $(OBJ) 
	gcc -lm -o $@ Main.o shell.o network.o
Main.o : Main.c Main.h
	gcc -c $*.c
shell.o : shell.c Main.h
	gcc -c $*.c
network.o : network.c Main.h network.h
	gcc -c $*.c
