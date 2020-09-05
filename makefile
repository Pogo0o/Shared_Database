CC			:= gcc
CCFLAGS		:= -Wall -g


all: clean so main test standalone

run: clean all

main:
	@echo "Building main..."
	$(CC) $(CCFLAGS) -o bin/main.o src/main.c -L ./lib/ -ldatabase -pthread -lrt

test:
	@echo "Building test..."
	$(CC) $(CCFLAGS) -o bin/test.o tests/main.c -L ./lib/ -ldatabase -pthread -lrt

standalone:
	@echo "Building standalone..."
	$(CC) $(CCFLAGS) -o bin/independent_main.o src/Database.c tests/standalone.c -pthread -lrt

so: 
	@echo "Building Shared Object..."
	$(CC) $(CCFLAGS) -shared -fpic -o lib/libdatabase.so src/Database.c -lrt
	sudo cp lib/libdatabase.so /usr/lib

clean:
	@echo "Clearing..."
	-rm bin/*
	-rm lib/*