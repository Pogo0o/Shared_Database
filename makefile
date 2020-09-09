CC			:= gcc
CCFLAGS		:= -Wall -g

all: dirs so consumer producer standalone

dirs:
	mkdir -p ./bin/
	mkdir -p ./lib/

consumer:
	@echo "Building main..."
	$(CC) $(CCFLAGS) -o bin/Consumer tests/Consumer.c -L./lib/ -ldatabase -pthread -lrt

producer:
	@echo "Building test..."
	$(CC) $(CCFLAGS) -o bin/Producer tests/Producer.c -L./lib/ -ldatabase -pthread -lrt

standalone:
	@echo "Building standalone..."
	$(CC) $(CCFLAGS) -o bin/Standalone src/Database.c tests/standalone.c -pthread -lrt

so: 
	@echo "Building Shared Object..."
	$(CC) $(CCFLAGS) -shared -fpic -o lib/libdatabase.so src/Database.c -lrt

clean:
	@echo "Clearing..."
	-rm bin/*
	-rm lib/*