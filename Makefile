CC := gcc
CFLAGS := -Wall -Wextra -g -fsanitize=address

ALEGRO_EXEC := frogger-allegro
RASPBERRY_EXEC := frogger-raspberry

.PHONY: allegro raspberry backend clean

default:
	@echo " Especificar \"allegro\" o \"raspberry\" "

allegro: backend
	@echo "Compilando frontend para Allegro..."
	$(CC) $(CFLAGS) -o $(ALEGRO_EXEC) frogger.c

raspberry: backend
	@echo "Compilando frontend para Raspberry PI..."
	$(CC) $(CFLAGS) -D RASPBERRY -o $(RASPBERRY_EXEC) frogger.c

##################################################################

backend:
	@echo "Compilando backend..."

##################################################################

clean:
	@echo "Cleaning..."
	rm *.o
