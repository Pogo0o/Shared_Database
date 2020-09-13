## Shared Database

### Introduction
This repository contains a simple dynamically shared library that implements a shareable binary database with a synchronization mechanism for all programs connected to it.

### Main Functionality
* Writing to a remote database
* Reading from a remote database
* Changing a record inside a local copy of the database
* Automatic synchronization of the local copy with the remote 

### Compilation
Use included makefile with a GNU compiler in order to build the project:
	
	make
The compilation output should be as follows:

	/lib/libdatabase.so
	./consumer
	./producer

### Testing
Inside of the project there are two files that were used to assure that the main functionality of the database are acheived correctly.
* Consumer program
* Producer program
##### Consumer
This program simulates the synchronization of the database each time a broadcast is received.
Output example:

	./consumer 
	Joining shared memory...
	No existing shared memory found...
	Initalizing thread synchronization...
	Initalizing remote database...
	Initialization COMPLETED
	Log number: 0
	Record 0 is now: 0
	Record 1 is now: 0
	Record 2 is now: 0
	Record 3 is now: 0
	Record 4 is now: 0
	Broadcast RECEIVED
	Synchronization COMPLETED

##### Producer
This program simulates writing to the database, it increases a record, of an index passed as a program argument, by one each time the program is called.
Output example:
	
	Joining shared memory...
	Initalizing remote database...
	Initialization COMPLETED
	Data was 0
	Local record changed...
	Data is 1

### Known Issues
 - [ ] This project does not implement a cleanup program that clears the shared memory segment and this causes problem once all instances of a database are closed, as pointers to data synchronization mutex and condition variables are freed. Reboot solves this problem as it makes sure that there is no shared memory allocated.
 - [x] FIXED. Deadlock occurs during the write operation of a producer. It was casued by a lack of a synchronization between a condition broadcast and condition wait, which resulted in infinite waiting from the consumer thread.
