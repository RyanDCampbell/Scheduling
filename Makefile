
CC		= gcc
CFLAGS		= -Wall -Wpedantic -Wextra -Werror
LDFLAGS		= -lpthread
OBJFILES	= scheduling.o task.o token.o mlfq.o #profiling.o
TARGET		= scheduling

all: $(TARGET)

$(TARGET): $(OBJFILES)
	$(CC) -o $(TARGET) $(OBJFILES) $(LDFLAGS) $(CFLAGS)

clean:
	rm -f $(OBJFILES) $(TARGET) *~

#scheduling: scheduling.c
#	cc scheduling.c -o scheduling -lpthread -lrt -Wall # -Wpedantic -Wextra -Werror

#clean:
#	rm -f scheduling
