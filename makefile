CC			:= gcc
CCFLAGS		:= -Wall -g
SHARED		:= -shared -fpic
LIBS		:= -ldatabase -pthread -lrt
DBLINK		:= -Wl,-rpath,"./lib/"

all: dirs libdatabase consumer producer cleaner

dirs:
	mkdir -p ./lib/

consumer: libdatabase
	@echo "Building main..."
	$(CC) $(CCFLAGS) -o $@ tests/Consumer.c -L./lib/ $(LIBS) $(DBLINK)

producer: libdatabase
	@echo "Building test..."
	$(CC) $(CCFLAGS) -o $@ tests/Producer.c -L./lib/ $(LIBS) $(DBLINK)

cleaner: libdatabase
	@echo "Building test..."
	$(CC) $(CCFLAGS) -o $@ tests/Cleaner.c -L./lib/ $(LIBS) $(DBLINK)

libdatabase: 
	@echo "Building Shared Object..."
	$(CC) $(CCFLAGS) $(SHARED) -o lib/$@.so src/Database.c -lrt

clean:
	@echo "Clearing..."
	-rm lib/*
	-rm ./cleaner ./consumer ./producer