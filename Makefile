CC = gcc
CFLAG = -c
DEBUG = -g

ALL.O = list.o DS_Struct.o utils.o main_server.o client_connect.o server.o client.o data_server.o

all: $(ALL.O)
	$(CC) main_server.c list.o DS_Struct.o utils.o server.o -o main_server.out
	$(CC) data_server.c list.o DS_Struct.o utils.o server.o client_connect.o -o data_server.out
	$(CC) client.c list.o DS_Struct.o utils.o client_connect.o -o client.out

list.o: list.c list.h
	$(CC) $(CFLAG) list.c

DS_Struct.o: DS_Struct.c DS_Struct.h list.o
	$(CC) $(CFLAG) DS_Struct.c

utils.o: utils.c utils.h list.o list.h DS_Struct.o DS_Struct.h
	$(CC) $(CFLAG) utils.c

main_server.o: main_server.c utils.h server.o
	$(CC) $(CFLAG) main_server.c

data_server.o: data_server.c utils.h server.o
	$(CC) $(CFLAG) data_server.c

server.o: server.c server.h utils.h utils.o
	$(CC) $(CFLAG) server.c

client.o: client.c client_connect.o utils.h utils.o
	$(CC) $(CFLAG) client.c

client_connect.o: client_connect.c client_connect.h utils.h utils.o
	$(CC) $(CFLAG) client_connect.c

clean:
	rm *.o *.out
