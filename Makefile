CC = gcc
CFLAGS = -Wall -Wextra -g -fsanitize=address -O2
LDFLAGS = -lm

all: render test

render: bitmap.o geometry.o raytracing.o render.o
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

test: geometry.o raytracing.o test.o
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@ 

clean:
	rm -f *.o render

