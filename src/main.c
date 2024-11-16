#include <time.h>
#include "screen.h"
#include "keyboard.h"
#include <unistd.h>
#include "menu.h"
#include <stdio.h>
#include <stdlib.h>

#define MAP_WIDTH 60   // Novo tamanho da largura do mapa
#define MAP_HEIGHT 30  // Novo tamanho da altura do mapa
#define MAX_ENEMIES 5 // Número de inimigos
#define MAX_FIREBALLS 5 // Número Maximo de Bolas de Fogo

// Cores para os elementos do mapa
#define COLOR_WALL RED
#define COLOR_DOOR YELLOW
#define COLOR_FLOOR LIGHTGRAY
#define COLOR_PLAYER GREEN
#define COLOR_ENEMY MAGENTA
#define COLOR_ATTACK CYAN
#define COLOR_BOMB YELLOW
#define COLOR_FIREBALL RED
#define ENEMY_MOVE_INTERVAL 1000
#define FIREBALL_MOVE_INTERVAL 100  

int player_lives = 3;  // Número de vidas do jogador

// Definição do mapa ampliado
char map[MAP_HEIGHT][MAP_WIDTH] = {
    "############################################################",
    "#                                                          #",
    "#                                                          #",
    "#                                                          #",
    "#                                                          #",
    "#                                                          #",
    "#                                                          #",
    "#                                                          #",
    "#                                                          #",
    "#                                                          #",
    "#                                                          #",
    "#                                                          #",
    "#                                                          #",
    "#                                                          #",
    "#                                                          #",
    "#                                                          #",
    "#                                                          #",
    "#                                                          #",
    "#                                                          #",
    "############################################################"
};

// Posição inicial do jogador
int playerX = 30;
int playerY = 10;

// Estrutura para os inimigos
typedef struct {
    int x, y;
    int alive;
} Enemy;

// Estrutura para as bolas de fogo
typedef struct {
    int x, y;
    int active;
    time_t createdAt;
    int direction;
} Fireball;

// Array de inimigos
Enemy enemies[MAX_ENEMIES];
Fireball fireballs[MAX_FIREBALLS];

void screenDrawMap();
void PrintMago();
void drawEnemies(); // Função para desenhar inimigos
void movePlayer(int dx, int dy);
int isOccupiedByEnemy(int x, int y); // Função para verificar se uma posição está ocupada por um inimigo
void moveEnemies(); // Função para mover inimigos
void showAttackFeedback();
void playerAttack();
void createFireball();
void initFireballs();
void updateFireballs(struct timespec *lastFireballMove);
void placeBombs(int num_bombs);
void check_bomb_collision(int player_x, int player_y);
void check_enemy_collision(int player_x, int player_y); // Função para verificar colisão com inimigos
void player_move(int new_x, int new_y);
void refreshScreen();
void drawPlayerLives();
void drawFireballs();
void initEnemies(); // Função para inicializar inimigos

int main() {
    displayOpeningArt();
    keyboardInit();
    screenInit(0);

    initFireballs();
    initEnemies(); // Inicializa inimigos
    placeBombs(10);

    refreshScreen();

    struct timespec lastEnemyMove, lastFireballMove, currentTime, lastScreenUpdate;
    clock_gettime(CLOCK_MONOTONIC, &lastEnemyMove);
    clock_gettime(CLOCK_MONOTONIC, &lastFireballMove);
    clock_gettime(CLOCK_MONOTONIC, &lastScreenUpdate);

    while (1) {
        // Verifica se há uma tecla pressionada
        if (keyhit()) {
            char key = readch();
            switch (key) {
                case 'w': movePlayer(0, -1); break;
                case 's': movePlayer(0, 1); break;
                case 'a': movePlayer(-1, 0); break;
                case 'd': movePlayer(1, 0); break;
                case 'f': createFireball(); break;
                case ' ': playerAttack(); break;
                case 'q':
                    keyboardDestroy();
                    screenDestroy();
                    return 0;
            }
        }

        updateFireballs(&lastFireballMove);

        // Verifica se o intervalo para mover inimigos foi atingido
        clock_gettime(CLOCK_MONOTONIC, &currentTime);
        long elapsed_ms = (currentTime.tv_sec - lastEnemyMove.tv_sec) * 1000 +
                          (currentTime.tv_nsec - lastEnemyMove.tv_nsec) / 1000000;
        if (elapsed_ms >= ENEMY_MOVE_INTERVAL) {
            moveEnemies(); // Move inimigos
            lastEnemyMove = currentTime;
        }

        // Verifica se a tela precisa ser atualizada
        elapsed_ms = (currentTime.tv_sec - lastScreenUpdate.tv_sec) * 1000 +
                     (currentTime.tv_nsec - lastScreenUpdate.tv_nsec) / 1000000;

        if (elapsed_ms >= 100) {  // Atualiza a tela a cada 100ms
            refreshScreen();
            lastScreenUpdate = currentTime;
        }

        usleep(1000);  // Delay de 1ms para evitar loop excessivo
    }
}

// Função que Atualiza a Tela
void refreshScreen() {
    screenDrawMap();  
    PrintMago();      
    drawEnemies(); // Desenha inimigos
    drawPlayerLives();
    drawFireballs(); // Adicione esta linha
    fflush(stdout);
}

// Função para desenhar a bola de fogo
void drawFireballs() {
    screenSetColor(COLOR_FIREBALL, BLACK);
    for (int i = 0; i < MAX_FIREBALLS; i++) {
        if (fireballs[i].active) {
            screenGotoxy(fireballs[i].x, fireballs[i].y);
            printf("🔥");
        }
    }
    screenSetColor(WHITE, BLACK);
}

// Função para desenhar o mapa
void screenDrawMap() {
    for (int y = 0; y < MAP_HEIGHT; y++) {
        for (int x = 0; x < MAP_WIDTH; x++) {
            char cell = map[y][x];  // Declare 'cell' aqui dentro do loop

            switch(cell) {
                case '#':
                    screenSetColor(COLOR_WALL, BLACK);
                    break;
                case 'D':
                    screenSetColor(COLOR_DOOR, BLACK);
                    break;
                case 'B':  // Desenhar bombas
                    screenSetColor(COLOR_BOMB, BLACK);
                    screenGotoxy(x, y);
                    printf("💣");  // Exibir emoji da bomba
                    screenSetColor(WHITE, BLACK);
                    continue;  // Evita sobrescrever abaixo
                default:
                    screenSetColor(COLOR_FLOOR, BLACK);
                    break;
            }

            // Aqui, o 'cell' está corretamente definido
            screenGotoxy(x, y);
            printf("%c", cell);  // Exibe o conteúdo da célula
        }
    }
    screenSetColor(WHITE, BLACK);
    fflush(stdout);
}

// Função para desenhar o jogador
void PrintMago() {
    screenSetColor(COLOR_PLAYER, BLACK);
    screenGotoxy(playerX, playerY);
    printf("🧙");
    fflush(stdout);
}

// Função para desenhar os inimigos
void drawEnemies() {
    screenSetColor(COLOR_ENEMY, BLACK);
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (enemies[i].alive) {
            screenGotoxy(enemies[i].x, enemies[i].y);
            printf("🦹");
        }
    }
    fflush(stdout);
}

// Função para mover o jogador
void movePlayer(int dx, int dy) {
    int newX = playerX + dx;
    int newY = playerY + dy;

    if (map[newY][newX] != '#') {  // Verifica se o destino é válido
        map[playerY][playerX] = ' ';  // Atualiza o mapa limpando a posição antiga
        playerX = newX;
        playerY = newY;
        check_bomb_collision(playerX, playerY);  // Verifica colisão com bombas
        check_enemy_collision(playerX, playerY); // Verifica colisão com inimigos
        map[playerY][playerX] = '@';  // Atualiza o mapa com a nova posição
        refreshScreen();  // Redesenha tudo
    }
}

// Função para verificar se uma posição está ocupada por outro inimigo
int isOccupiedByEnemy(int x, int y) {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (enemies[i].alive && enemies[i].x == x && enemies[i].y == y) {
            return 1;
        }
    }
    return 0;
}

// Função para mover inimigos em direção ao jogador
void moveEnemies() {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (!enemies[i].alive) continue;

        screenGotoxy(enemies[i].x, enemies[i].y);
        printf(" ");  // Limpa a posição antiga

        int dx = (playerX > enemies[i].x) ? 1 : (playerX < enemies[i].x) ? -1 : 0;
        int dy = (playerY > enemies[i].y) ? 1 : (playerY < enemies[i].y) ? -1 : 0;

        int newX = enemies[i].x + dx;
        int newY = enemies[i].y + dy;

        if (map[newY][newX] != '#' && !isOccupiedByEnemy(newX, newY)) {
            enemies[i].x = newX;
            enemies[i].y = newY;
        }

        // Verifica colisão com o jogador
        if (enemies[i].x == playerX && enemies[i].y == playerY) {
            check_enemy_collision(playerX, playerY);
        }
    }
    drawEnemies();  // Atualiza os inimigos na tela
}

// Função para exibir feedback visual do ataque ao redor do jogador
void showAttackFeedback() {
    screenSetColor(COLOR_ATTACK, BLACK);
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            if (dx == 0 && dy == 0) continue;

            int x = playerX + dx;
            int y = playerY + dy;
            if (x >= 0 && x < MAP_WIDTH && y >= 0 && y < MAP_HEIGHT && map[y][x] != '#') {
                screenGotoxy(x, y);
                printf("*");
            }
        }
    }
    fflush(stdout);
}

// Função para ataque com feedback visual
void playerAttack() {
    showAttackFeedback();

    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (!enemies[i].alive) continue;

        if (abs(enemies[i].x - playerX) <= 1 && abs(enemies[i].y - playerY) <= 1) {
            screenGotoxy(enemies[i].x, enemies[i].y);
            printf(" ");
            enemies[i].alive = 0;
        }
    }

    screenGotoxy(playerX, playerY);
    PrintMago();
    
    screenDrawMap();
    drawEnemies();
}

// Inicializa as bolas de fogo
void initFireballs() {
    for (int i = 0; i < MAX_FIREBALLS; i++) {
        fireballs[i].active = 0;
    }
}

// Cria uma nova bola de fogo
void createFireball() {
    for (int i = 0; i < MAX_FIREBALLS; i++) {
        if (!fireballs[i].active) {
            // Limpa a posição anterior da bola de fogo
            screenGotoxy(fireballs[i].x, fireballs[i].y);
            printf(" ");
            
            // Define a posição inicial da bola de fogo uma posição à frente do mago
            fireballs[i].x = playerX + 1;
            fireballs[i].y = playerY;
            
            // Verifica se a posição é válida (não é parede)
            if (map[fireballs[i].y][fireballs[i].x] == '#') {
                continue;  // Não cria a bola de fogo se estiver bloqueada por parede
            }
            
            fireballs[i].active = 1;
            fireballs[i].createdAt = time(NULL);
            fireballs[i].direction = 1;
            
            // Desenha a bola de fogo em sua posição inicial
            screenSetColor(COLOR_FIREBALL, BLACK);
            screenGotoxy(fireballs[i].x, fireballs[i].y);
            printf("🔥");
            screenSetColor(WHITE, BLACK);
            
            // Mantém o mago em sua posição
            PrintMago();
            break;
        }
    }
}

// Atualiza a posição das bolas de fogo e verifica colisões
void updateFireballs(struct timespec *lastFireballMove) {
    struct timespec currentTime;
    clock_gettime(CLOCK_MONOTONIC, &currentTime);
    long elapsed_ms = (currentTime.tv_sec - lastFireballMove->tv_sec) * 1000 +
                      (currentTime.tv_nsec - lastFireballMove->tv_nsec) / 1000000;

    if (elapsed_ms < FIREBALL_MOVE_INTERVAL) {
        return;
    }
    *lastFireballMove = currentTime;

    for (int i = 0; i < MAX_FIREBALLS; i++) {
        if (fireballs[i].active) {
            // Limpa a posição anterior
            screenGotoxy(fireballs[i].x, fireballs[i].y);
            printf(" ");  

            int newX = fireballs[i].x + fireballs[i].direction;
            
            // Verifica colisão com parede
            if (map[fireballs[i].y][newX] == '#') {
                fireballs[i].active = 0;
                continue;
            }

            // Verifica colisão com inimigos
            for (int j = 0; j < MAX_ENEMIES; j++) {
                if (enemies[j].alive && enemies[j].x == newX && enemies[j].y == fireballs[i].y) {
                    enemies[j].alive = 0;
                    screenGotoxy(enemies[j].x, enemies[j].y);
                    printf(" ");
                    fireballs[i].active = 0;
                    break;
                }
            }

            // Atualiza posição da bola de fogo
            if (fireballs[i].active) {
                fireballs[i].x = newX;
            }
        }
    }
    
    // Força um refresh completo da tela após as atualizações
    refreshScreen();
}

// Função para posicionar bombas aleatoriamente no mapa
void placeBombs(int num_bombs) {
    srand(time(NULL));
    for (int i = 0; i < num_bombs; i++) {
        int x, y;
        do {
            x = rand() % MAP_WIDTH;
            y = rand() % MAP_HEIGHT;
        } while (map[y][x] != ' ');  // Garantir que não sobrescreva paredes, inimigos ou o jogador

        map[y][x] = 'B';  // Representação interna da bomba no mapa
    }
}

// Função para verificar colisão com bombas
void check_bomb_collision(int x, int y) {
    if (map[y][x] == 'B') {
        player_lives--;
        printf("Você pisou em uma bomba! Vidas restantes: %d\n", player_lives);
        map[y][x] = ' ';  // Remove a bomba do mapa
        if (player_lives <= 0) {
            printf("Game Over!\n");
            exit(0);  // Termina o jogo
        }
    }
}

// Função para verificar colisão com inimigos
void check_enemy_collision(int x, int y) {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (enemies[i].alive && enemies[i].x == x && enemies[i].y == y) {
            player_lives--;
            printf("Você foi atingido por um inimigo! Vidas restantes: %d\n", player_lives);
            if (player_lives <= 0) {
                printf("Game Over!\n");
                exit(0);  // Termina o jogo
            }
        }
    }
}

// Função Para desenhar a Vida atual do Jogador
void drawPlayerLives() {
    screenSetColor(WHITE, BLACK);  // Definir a cor para o texto (branco no fundo preto)
    screenGotoxy(2, 1);  // Posiciona no canto superior esquerdo (linha 1, coluna 2)
    printf("Vidas: %d", player_lives);  // Exibe a quantidade de vidas
}

void player_move(int new_x, int new_y) {
    check_bomb_collision(new_x, new_y);
    check_enemy_collision(new_x, new_y); // Verifica colisão com inimigos
    movePlayer(new_x - playerX, new_y - playerY);
}

// Função para inicializar inimigos em posições aleatórias
void initEnemies() {
    srand(time(NULL));
    for (int i = 0; i < MAX_ENEMIES; i++) {
        int x, y;
        do {
            x = rand() % MAP_WIDTH;
            y = rand() % MAP_HEIGHT;
        } while (map[y][x] != ' ');  // Garantir que não sobrescreva paredes, bombas ou o jogador

        enemies[i].x = x;
        enemies[i].y = y;
        enemies[i].alive = 1;
    }
}