CC := gcc
# Agregamos -Ibackend y -Ifrontend para que encuentre los archivos .h solos
CFLAGS := -Wall -Wextra -g -fsanitize=address -Ibackend -Ifrontend

ALLEGRO_EXEC := frogger-allegro
RASPBERRY_EXEC := frogger-raspberry

# Le aclaramos en qué carpeta está cada cosa
BACKEND_SRC := main.c backend/juego.c 

ALLEGRO_SRC := frontend/allegro/frontendAllegro.c
RASPBERRY_SRC := frontend/raspberry/frontendRaspberry.c

.PHONY: default allegro raspberry clean

default:
	@echo "Especificar \"make allegro\" o \"make raspberry\""

allegro:
	@echo "Compilando versión para Allegro..."
	$(CC) $(CFLAGS) $(BACKEND_SRC) $(ALLEGRO_SRC) -o $(ALLEGRO_EXEC) -lallegro -lallegro_primitives -lallegro_image -lallegro_font -lallegro_ttf

raspberry:
	@echo "Compilando versión para Raspberry PI..."
	$(CC) $(CFLAGS) -D RASPBERRY $(BACKEND_SRC) $(RASPBERRY_SRC) -o $(RASPBERRY_EXEC)

clean:
	@echo "Limpiando ejecutables..."
	rm -f $(ALLEGRO_EXEC) $(RASPBERRY_EXEC)