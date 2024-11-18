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
        "                                                                                ____ ",
        "                                                                          .'* *.'",
        "                                                                       __/_*_*(_",
        "                                                                       / _______ \\",
        "                                                                     _\\_)/___\\(_/_ ",
        "                                                                    / _((\\- -/))_ \\",
        "                                                                    \\ \\())(-)(()/ /",
        "                                                                     ' \\(((()))/ '",
        "                                                                    / ' \\)).))/ ' \\",
        "                                                                   / _ \\ - | - /_  \\",
        "                                                                  (   ( .;''';. .'  )",
        "                                                                  _\"__ /    )\\ __\"/_",
        "                                                                    \\/  \\   ' /  \\/",
        "                                                                     .'  '...' ' )",
        "                                                                      / /  |  \\ \\",
        "                                                                     / .   .   . \\",
        "                                                                    /   .     .   \\",
        "                                                                   /   /   |   \\   \\",
        "                                                                 .'   /    b    '.  '.",
        "                                                             _.-'    /     Bb     '-. '-._ ",
        "                                                         _.-'       |      BBb       '-.  '-. ",
        "                                                        (________mrf\\____.dBBBb.________)____)"
        "                                                                                             ",
        "                                                                 Bem Vindo ao Mage Game!",
        "                                                       Sobreviva o Maximo de Tempo Possivel no Mapa",
        "                                                                    W A S D para mover",
        "                                                                 I J K L  para disparar magia",
        "                                                                Pressione ENTER para continuar...                ",
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