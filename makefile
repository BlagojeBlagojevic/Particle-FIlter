CC = gcc
CFLAGS = -Wall -Wextra -Og -ggdb
RAYLIBFLAGS = -Iraylib/include raylib/lib/libraylib.a -lGL -lm -lpthread -ldl -lrt -lX11
TARGET = particle
OBJS = particle.o kalman.o main.o

$(TARGET): $(OBJS)
	$(CC) $(OBJS) $(RAYLIBFLAGS) -o $(TARGET)

main.o: main.c
	$(CC) $(CFLAGS) -c main.c

particle.o: particle.c particle.h
	$(CC) $(CFLAGS) -c particle.c

kalman.o: kalman.c kalman.h
	$(CC) $(CFLAGS) -c kalman.c

.PHONY: clean
clean:
	rm -f $(TARGET) $(OBJS)