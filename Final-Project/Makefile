# Specify the target executable and the source files needed to build it
symnmf: symnmf.o symnmf.h
	gcc -o symnmf symnmf.o -lm
# Specify the object files that are generated from the corresponding source files
symnmf.o: symnmf.c
# 	gcc -c symnmf.c
	gcc -ansi -Wall -Wextra -Werror -lm -c -pedantic-errors symnmf.c
# TODO: i manually added -lm and -c		--> make sure its okay!
<!-- symnmfmodule.o: symnmfmodule.c
	gcc -c symnmfmodule.c -->