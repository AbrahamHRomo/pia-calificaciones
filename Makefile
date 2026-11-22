CC      ?= cc
CFLAGS  ?= -std=c11 -Wall -Wextra -Wpedantic -O2
TARGET   = pia
SRC      = PIA.c
OBJ      = $(SRC:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

run: $(TARGET)
	./$(TARGET) ejemplo_calificaciones.csv reporte.csv

clean:
	rm -f $(TARGET) $(OBJ) reporte.csv

.PHONY: all run clean
