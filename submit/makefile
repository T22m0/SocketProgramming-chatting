OBJ = issaccho_proj1.o shell.o network.o
PA1 : $(OBJ) 
	gcc -lm -o $@ issaccho_proj1.o shell.o network.o
issaccho_proj1.o : issaccho_proj1.c network.h
	gcc -c $*.c
shell.o : shell.c network.h
	gcc -c $*.c
network.o : network.c network.h
	gcc -c $*.c
