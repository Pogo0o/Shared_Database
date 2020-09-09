CC			:= gcc
CCFLAGS		:= -Wall -g

all: dirs so main test standalone

dirs:
	mkdir -p ./bin/
	mkdir -p ./lib/

main:
	@echo "Building main..."
	$(CC) $(CCFLAGS) -o bin/main src/main.c -L./lib/ -ldatabase -pthread -lrt

test:
	@echo "Building test..."
	$(CC) $(CCFLAGS) -o bin/test tests/main.c -L./lib/ -ldatabase -pthread -lrt

standalone:
	@echo "Building standalone..."
	$(CC) $(CCFLAGS) -o bin/independent_main src/Database.c tests/standalone.c -pthread -lrt

so: 
	@echo "Building Shared Object..."
	$(CC) $(CCFLAGS) -shared -fpic -o lib/libdatabase.so src/Database.c -lrt

clean:
	@echo "Clearing..."
	-rm bin/*
	-rm lib/*