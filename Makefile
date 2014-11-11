CC=cc -DDEBUG
CFLAGS = -Wall 

all: compile

compile: server taskManager

server: server.o packetManager.o clientDataStorage.o utils.o taskManager.o
	$(CC) $^ $(CFLAGS) -o $@
	
taskManager: client.o packetManager.o utils.o
	$(CC) $^ $(CFLAGS) -o $@

run: taskManager
	./taskManager
