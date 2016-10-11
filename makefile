OBJ = Main.o shell.o network.o
PA1 : $(OBJ) 
	gcc -lm -o $@ Main.o shell.o network.o
Main.o : Main.c network.h
	gcc -c $*.c
shell.o : shell.c network.h
	gcc -c $*.c
network.o : network.c network.h
	gcc -c $*.c
