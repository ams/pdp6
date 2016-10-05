SRC=main.c apr.c mem.c tty.c
# clang
#CFLAGS= -Wno-shift-op-parentheses -Wno-logical-op-parentheses \
#        -Wno-bitwise-op-parentheses
CFLAGS=  -fno-diagnostics-show-caret \
        -L/usr/local/lib -I/usr/local/include -lSDL -lSDL_image -g -lpthread


pdp6: $(SRC) pdp6.h
	$(CC) $(CFLAGS) $(SRC) -o pdp6

