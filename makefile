CC			:= gcc
CCFLAGS		:= -Wall -g

all: dirs so consumer producer linker

dirs:
	mkdir -p ./bin/
	mkdir -p ./lib/

consumer:
	@echo "Building main..."
	$(CC) $(CCFLAGS) -o bin/Consumer tests/Consumer.c -L./lib/ -ldatabase -pthread -lrt

producer:
	@echo "Building test..."
	$(CC) $(CCFLAGS) -o bin/Producer tests/Producer.c -L./lib/ -ldatabase -pthread -lrt

so: 
	@echo "Building Shared Object..."
	$(CC) $(CCFLAGS) -shared -fpic -o lib/libdatabase.so src/Database.c -lrt

clean:
	@echo "Clearing..."
	-rm bin/*
	-rm lib/*