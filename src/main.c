#include <time.h>
#include "screen.h"
#include "keyboard.h"
#include <unistd.h>
#include "menu.h"
#include <stdio.h>
#include <stdlib.h>

#define MAP_WIDTH 60   // Novo tamanho da largura do mapa
#define MAP_HEIGHT 30  // Novo tamanho da altura do mapa
#define MAX_ENEMIES 10 // Número de inimigos
#define MAX_BOSS 1 // Número de inimigos
#define MAX_FIREBALLS 5 // Número Maximo de Bolas de Fogo
#define BOSS_MOVE_INTERVAL 200


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
#define FIREBALL_MOVE_INTERVAL 50  

int player_lives = 3;  // Número de vidas do jogador
int enemies_defeated = 0;  // Track number of defeated enemies
int boss_health = 10;     // Boss health points
int boss_spawned = 0; 

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

// Estrutura para os inimigos
typedef struct {
    int x, y;
    int alive;
    int health;
} Boss;


// Estrutura para as bolas de fogo
typedef struct {
    int x, y;
    int active;
    time_t createdAt;
    int direction;
} Fireball;

// Array de inimigos
Enemy enemies[MAX_ENEMIES];
Boss boss[MAX_BOSS];
Fireball fireballs[MAX_FIREBALLS];

void screenDrawMap();
void PrintMago();
void drawEnemies(); // Função para desenhar inimigos
void movePlayer(int dx, int dy);
int  isOccupiedByEnemy(int x, int y); // Função para verificar se uma posição está ocupada por um inimigo
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
void createFireballInDirection(int direction);
void handleFireballInput(char key);
void drawBoss();
void checkBossSpawn();
void check_boss_collision(int x, int y);
void moveBoss();
void print_ascii(char *path);

// Função Main
int main() {
    displayOpeningArt();
    keyboardInit();
    screenInit(0);

    initFireballs();
    initEnemies();
    placeBombs(10);

    refreshScreen();

    struct timespec lastEnemyMove, lastFireballMove, lastBossMove, currentTime, lastScreenUpdate;
    clock_gettime(CLOCK_MONOTONIC, &lastEnemyMove);
    clock_gettime(CLOCK_MONOTONIC, &lastFireballMove);
    clock_gettime(CLOCK_MONOTONIC, &lastBossMove);
    clock_gettime(CLOCK_MONOTONIC, &lastScreenUpdate);

    while (1) {
        if (keyhit()) {
            char key = readch();
            switch (key) {
                case 'w': movePlayer(0, -1); break;
                case 's': movePlayer(0, 1); break;
                case 'a': movePlayer(-1, 0); break;
                case 'd': movePlayer(1, 0); break;
                case 'i': // Fireball up
                case 'k': // Fireball down
                case 'j': // Fireball left
                case 'l': // Fireball right
                    handleFireballInput(key); 
                    break;
                case ' ': playerAttack(); break;
                case 'q':
                    keyboardDestroy();
                    screenDestroy();
                    return 0;
            }
        }

        updateFireballs(&lastFireballMove);

        // Obtém o tempo atual
        clock_gettime(CLOCK_MONOTONIC, &currentTime);

        // Verifica movimentação dos inimigos
        long elapsed_ms_enemy = (currentTime.tv_sec - lastEnemyMove.tv_sec) * 1000 +
                               (currentTime.tv_nsec - lastEnemyMove.tv_nsec) / 1000000;
        if (elapsed_ms_enemy >= ENEMY_MOVE_INTERVAL) {
            moveEnemies();
            lastEnemyMove = currentTime;
        }

        // Verifica movimentação do boss
        long elapsed_ms_boss = (currentTime.tv_sec - lastBossMove.tv_sec) * 1000 +
                              (currentTime.tv_nsec - lastBossMove.tv_nsec) / 1000000;
        if (elapsed_ms_boss >= BOSS_MOVE_INTERVAL && boss_spawned) {
            moveBoss(&lastBossMove);
            lastBossMove = currentTime;
        }

        // Atualiza a tela
        long elapsed_ms_screen = (currentTime.tv_sec - lastScreenUpdate.tv_sec) * 1000 +
                                (currentTime.tv_nsec - lastScreenUpdate.tv_nsec) / 1000000;

        if (elapsed_ms_screen >= 100) {
            refreshScreen();
            lastScreenUpdate = currentTime;
        }
        usleep(1000);
    }
}

void print_ascii(char *path)
{
    FILE *file;
    char string[128];

    file = fopen(path, "r");

    if (file == NULL)
    {
        printf("Error opening file");
        return;
    }

    while (fgets(string, sizeof(string), file) != NULL)
    {
        printf("%s", string);
    }

    fclose(file);
}

// Função para criar uma bola de fogo em uma direção específica
void createFireballInDirection(int direction) {
    for (int i = 0; i < MAX_FIREBALLS; i++) {
        if (!fireballs[i].active) {  // Find an inactive fireball
            fireballs[i].x = playerX;
            fireballs[i].y = playerY;
            fireballs[i].active = 1;
            fireballs[i].direction = direction;
            break;
        }
    }
}

// Função para lidar com a entrada do jogador para atirar a bola de fogo
void handleFireballInput(char key) {
    switch (key) {
        case 'i': createFireballInDirection(0); break;  // Atira para cima
        case 'k': createFireballInDirection(1); break;  // Atira para baixo
        case 'j': createFireballInDirection(-1); break; // Atira para a esquerda
        case 'l': createFireballInDirection(2); break;  // Atira para a direita
    }
}

// Função que Atualiza a Tela
void refreshScreen() {
    screenDrawMap();
    PrintMago();
    drawEnemies();
    if (boss_spawned) {
        drawBoss();
    }
    drawPlayerLives();
    drawFireballs();
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
                    printf("B");  // Exibir emoji da bomba
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

// Função para desenhar o boss
void drawBoss() {
    screenSetColor(COLOR_ENEMY, BLACK);
    for (int i = 0; i < MAX_BOSS; i++) {
        if (boss[i].alive) {
            screenGotoxy(boss[i].x, boss[i].y);
            printf("🧟");  // Boss emoji
            
            // Draw boss health bar
            screenGotoxy(2, 2);
            printf("Boss HP: ");
            for (int hp = 0; hp < boss[i].health; hp++) {
                printf("♥");
            }
        }
    }
    screenSetColor(WHITE, BLACK);
}

// Função Para checar se o boss nasceu
void checkBossSpawn() {
    if (enemies_defeated >= 10 && !boss_spawned) {
        for (int i = 0; i < MAX_BOSS; i++) {
            if (!boss[i].alive) {
                boss[i].alive = 1;
                boss[i].health = 10;
                boss_spawned = 1;
                
                // Limpa o centro do mapa para o boss
                int centerX = MAP_WIDTH / 2;
                int centerY = MAP_HEIGHT / 2;
                boss[i].x = centerX;
                boss[i].y = centerY;
                
                // Mensagem dramática de spawn do boss
                screenGotoxy(MAP_WIDTH/2 - 20, MAP_HEIGHT/2 - 5);
                printf("⚠️ O BOSS FINAL APARECEU! PREPARE-SE PARA A BATALHA! ⚠️");
                sleep(2); // Pausa dramática
                break;
            }
        }
       
    }
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

        check_bomb_collision(playerX, playerY);

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

// Função para verificar se uma posição está ocupada pelo boss
int isOccupiedByBoss(int x, int y) {
    for (int i = 0; i < MAX_BOSS; i++) {
        if (boss[i].alive && boss[i].x == x && boss[i].y == y) {
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

// Função para mover o boss em direção ao jogador
void moveBoss() {
    for (int i = 0; i < MAX_BOSS; i++) {
        if (!boss[i].alive) continue;

        screenGotoxy(boss[i].x, boss[i].y);
        printf(" ");  // Limpa a posição antiga

        int dx = (playerX > boss[i].x) ? 1 : (playerX < boss[i].x) ? -1 : 0;
        int dy = (playerY > boss[i].y) ? 1 : (playerY < boss[i].y) ? -1 : 0;

        int newX = boss[i].x + dx;
        int newY = boss[i].y + dy;

        if (map[newY][newX] != '#' && !isOccupiedByBoss(newX, newY)) {
            boss[i].x = newX;
            boss[i].y = newY;
        }

        // Verifica colisão com o jogador
        if (boss[i].x == playerX && boss[i].y == playerY) {
            check_boss_collision(playerX, playerY);
        }
    }
    drawBoss();  // Atualiza os inimigos na tela
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
            enemies_defeated++;  // Increment counter when enemy is defeated
            checkBossSpawn();    // Check if boss should spawn
        }
    }

    screenGotoxy(playerX, playerY);
    PrintMago();
    
    screenDrawMap();
    drawEnemies();
    if (boss_spawned) {
        drawBoss();
    }
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
            // Clear previous position
            screenGotoxy(fireballs[i].x, fireballs[i].y);
            printf(" ");

            int newX = fireballs[i].x;
            int newY = fireballs[i].y;

            // Update fireball position based on its direction
            switch (fireballs[i].direction) {
                case 0: newY -= 1; break; // Move up
                case 1: newY += 1; break; // Move down
                case -1: newX -= 1; break; // Move left
                case 2: newX += 1; break; // Move right
            }

            // Check for wall collisions
            if (newY < 0 || newY >= MAP_HEIGHT || newX < 0 || newX >= MAP_WIDTH || map[newY][newX] == '#') {
                fireballs[i].active = 0;
                continue;
            }

            // Check for enemy collisions
            for (int j = 0; j < MAX_ENEMIES; j++) {
                if (enemies[j].alive && enemies[j].x == newX && enemies[j].y == newY) {
                    enemies[j].alive = 0;
                    enemies_defeated++;
                    fireballs[i].active = 0;
                    checkBossSpawn(); // Check if the boss should spawn
                    screenGotoxy(enemies[j].x, enemies[j].y);
                    printf(" ");
                    break;
                }
            }

            // Check for boss collisions
            for (int j = 0; j < MAX_BOSS; j++) {
                if (boss[j].alive && boss[j].x == newX && boss[j].y == newY) {
                    boss[j].health--;
                    fireballs[i].active = 0;
                    if (boss[j].health <= 0) {
                        boss[j].alive = 0;
                        boss_spawned = 0;
                        screenGotoxy(boss[j].x, boss[j].y);
                        printf(" ");
                        // Victory message
                        screenGotoxy(MAP_WIDTH / 2 - 10, MAP_HEIGHT / 2);
                        printf("🎉 BOSS DEFEATED! 🎉");
                    }
                    break;
                }
            }

            // Update fireball position if still active
            if (fireballs[i].active) {
                fireballs[i].x = newX;
                fireballs[i].y = newY;
            }
        }
    }

    refreshScreen(); // Refresh the screen after updating fireballs
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
    // Verifica em todas as direções próximas ao jogador
    int bomb_coords[9][2] = {
        {x, y},     // Posição atual
        {x-1, y},   // Esquerda
        {x+1, y},   // Direita
        {x, y-1},   // Cima
        {x, y+1},   // Baixo
        {x-1, y-1}, // Superior esquerdo
        {x+1, y-1}, // Superior direito
        {x-1, y+1}, // Inferior esquerdo
        {x+1, y+1}  // Inferior direito
    };

    for (int i = 0; i < 9; i++) {
        int check_x = bomb_coords[i][0];
        int check_y = bomb_coords[i][1];

        // Verifica limites do mapa para evitar segmentation fault
        if (check_x >= 0 && check_x < MAP_WIDTH && 
            check_y >= 0 && check_y < MAP_HEIGHT) {
            
            if (map[check_y][check_x] == 'B') {
                player_lives--;
                printf("Você pisou em uma bomba! Vidas restantes: %d\n", player_lives);

                // Remove a bomba do mapa
                map[check_y][check_x] = ' ';

                // Verifica se o jogador ainda está vivo
                if (player_lives <= 0) {
                    print_ascii("files/death.txt");
                    exit(0);  // Termina o jogo
                }

                // Sai do loop após encontrar a primeira bomba
                break;
            }
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
                print_ascii("files/death.txt");
                exit(0);  // Termina o jogo
            }
        }
    }
}

// Função para verificar colisão com inimigos
void check_boss_collision(int x, int y) {
    for (int i = 0; i < MAX_BOSS; i++) {
        if (boss[i].alive && boss[i].x == x && boss[i].y == y) {
            player_lives--;
            printf("Você foi atingido por um inimigo! Vidas restantes: %d\n", player_lives);
            if (player_lives <= 0) {
                print_ascii("files/death.txt");
                exit(0);  // Termina o jogo
            }
        }
    }
}

// Função Para desenhar a Vida atual do Jogador
void drawPlayerLives() {
    screenSetColor(WHITE, BLACK);
    screenGotoxy(2, 1);
    printf("Vidas: ");
    for (int i = 0; i < player_lives; i++) {
        printf("♥");
    }
}

void player_move(int new_x, int new_y) {
    check_bomb_collision(new_x, new_y);
    check_enemy_collision(new_x, new_y); // Verifica colisão com inimigos
    movePlayer(new_x - playerX, new_y - playerY);
}

// Função para inicializar inimigos em posições aleatórias
void initEnemies() {
    srand(time(NULL));
    // Initialize regular enemies
    for (int i = 0; i < MAX_ENEMIES; i++) {
        int x, y;
        do {
            x = rand() % MAP_WIDTH;
            y = rand() % MAP_HEIGHT;
        } while (map[y][x] != ' ');

        enemies[i].x = x;
        enemies[i].y = y;
        enemies[i].alive = 1;
    }

    // Initialize boss (not alive initially)
    for (int i = 0; i < MAX_BOSS; i++) {
        boss[i].x = MAP_WIDTH / 2;
        boss[i].y = MAP_HEIGHT / 2;
        boss[i].alive = 0;
        boss[i].health = 10;
    }
}
