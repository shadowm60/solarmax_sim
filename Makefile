LIBS  = 
CFLAGS = -Wall -g3

# Should be equivalent to your list of C files, if you don't build selectively
SRC=$(wildcard *.c)

solarmax_sim: $(SRC)
	gcc -o $@ $^ $(CFLAGS) $(LIBS)