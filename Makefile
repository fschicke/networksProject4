#Thomas Clare (tclare) and Francis Schickel (fschicke)
all: netpong

netpong: netpong.o 
	gcc netpong.o  -o netpong -lncurses -lpthread -Wall

%.o: %.c *.h
	gcc -Wall -c $< -o $@

clean: 
	rm -f *.o netpong 
