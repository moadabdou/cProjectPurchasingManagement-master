
# Compiler
CC = gcc

# Executable name
EXEC = ./app

# Source files (app.c plus all other .c files in the current directory)
SRCS =  $(wildcard src/**/*.c)  $(wildcard src/*.c)

# Link the object files to create the executable
app: 
	$(CC) -g -o $(EXEC) $(SRCS) -lWs2_32 
	gdb -ex "run" -ex "bt" $(EXEC)
capp:
	$(CC) -o $(EXEC) $(SRCS) -lWs2_32 && ./app 
# Clean up the build
clean:
	rm $(EXEC) 

# Run the program
run: 
	$(EXEC)
