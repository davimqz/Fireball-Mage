#include "screen.h"
#include "menu.h"
#include "keyboard.h"

void displayMenu() {
    screenClear();

    // Espera o usuário pressionar ENTER para continuar
    while (1) {
        if (keyhit()) {
            char key = readch();
            if (key == '\n' || key == '\r') {
                fflush(stdout);
                screenClear();
                break;
            }
        }
    }
}

void displayOpeningArt() {
    screenClear();
    char *ascii_art[] = {
    "⠀⠀                                                                 Bem Vindo ao Mage Game!",
    "                                                           Sobreviva o Maximo de Tempo Possivel no Mapa",
    "                                                                       WASD para mover",
    "                                                                    F para disparar magia",
    "                                                                 Pressione ENTER para continuar...                ",
    };

    for (int i = 0; i < sizeof(ascii_art) / sizeof(ascii_art[0]); i++) {
        printf("%s\n", ascii_art[i]);
    }

    while (1) {
        if (keyhit()) {
            char key = readch();
            if (key == '\n' || key == '\r') {
                screenClear();
                break;
            }
        }
}
}