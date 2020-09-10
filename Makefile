# the compiler: gcc for C program, define as g++ for C++
CC = gcc
# compiler flags:
#  -g    adds debugging information to the executable file
#  -Wall turns on most, but not all, compiler warnings
CFLAGS = -g -Wall

# the build target executable:
TARGET = clientEXE awsEXE serverAEXE serverBEXE

all: $(TARGET)

serverAEXE: serverA.c
	$(CC) $(CFLAGS) -o serverAO serverA.c

serverBEXE: serverB.c
	$(CC) $(CFLAGS) -o serverBO serverB.c

awsEXE: aws.c
	$(CC) $(CFLAGS) -o awsO aws.c

clientEXE: client.c
	$(CC) $(CFLAGS) -o client client.c

serverA:
	./serverAO

aws:
	./awsO

serverB:
	./serverBO

clean:
	$(RM) $(TARGET)

