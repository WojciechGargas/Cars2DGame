#include <stdio.h>
#include <stdlib.h>

#include <time.h>
#include <fcntl.h>
#include <stdbool.h>

#include <termios.h>
#include <unistd.h>

#define WIDTH 14
#define HEIGHT 10
#define BASE_SPEED 300000

void pauseGame()
{
    printf("Gra zatrzymana. Naciśnij dowolny klawisz, aby wznowić...\n");
    getchar();
    return;
}
int getch()
{
    struct termios oldt, newt;
    int ch;

    tcgetattr(STDIN_FILENO, &oldt); // Pobierz aktualne ustawienia terminala
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);        // Wyłącz kanoniczny tryb i echo
    tcsetattr(STDIN_FILENO, TCSANOW, &newt); // Ustaw nowy tryb

    ch = getchar(); // Odczytaj jeden znak

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt); // Przywróć stare ustawienia
    return ch;
}

void display_tab(char tab[HEIGHT][WIDTH])
{
    for (int y = 0; y < HEIGHT; y++)
    {
        for (int x = 0; x < WIDTH; x++)
            putchar(tab[y][x]);
        putchar('\n');
    }
}

void frame(char tab[HEIGHT][WIDTH])
{
    tab[0][0] = '+';
    for (int x = 1; x < WIDTH; x++)
        tab[0][x] = '-';
    tab[0][WIDTH - 1] = '+';
    for (int y = 1; y < HEIGHT - 1; y++)
    {
        tab[y][0] = '|';
        tab[y][WIDTH - 1] = '|';
    }
    tab[HEIGHT - 1][0] = '+';
    for (int x = 1; x < WIDTH; x++)
        tab[HEIGHT - 1][x] = '-';
    tab[HEIGHT - 1][WIDTH - 1] = '+';
}

void spawn_item(char tab[HEIGHT][WIDTH], char object, int probability)
{
    int shouldSpawn = 1 + (rand() % probability);

    if (shouldSpawn == 1)
    {
        int randomObsPosition = 1 + (rand() % (WIDTH - 2));
        tab[1][randomObsPosition] = object;
    }
}
/*
void Spawn_obsticles(char tab[HEIGHT][WIDTH])
{
    int shouldSpawn = 1 + (rand() % 2);

    if(shouldSpawn == 1)
    {
        int randomObsPosition = 1 + (rand() % (WIDTH - 2));
        tab[1][randomObsPosition] = '#';
    }
}

void Spawn_bonus(char tab[HEIGHT][WIDTH])
{
    int shouldSpawn = 1 + (rand() % 3);

    if(shouldSpawn == 1)
    {
        int randomObsPosition = 1 + (rand() % (WIDTH - 2));
        tab[1][randomObsPosition] = '+';
    }
}

void Spawn_SpeedBoost(char tab[HEIGHT][WIDTH])
{
    int shouldSpawn = 1 + (rand() % 7);

    if(shouldSpawn == 1)
    {
        int randomObsPosition = 1 + (rand() % (WIDTH - 2));
        tab[1][randomObsPosition] = '^';
    }
}

void Spawn_Immortality(char tab[HEIGHT][WIDTH])
{
    int shouldSpawn = 1 + (rand() % 10);

    if(shouldSpawn == 1)
    {
        int randomObsPosition = 1 + (rand() % (WIDTH - 2));
        tab[1][randomObsPosition] = 'i';
    }
}
*/
void move_player(char tab[HEIGHT][WIDTH], int x)
{
    for (int i = 1; i < WIDTH - 1; i++)
    {
        if (tab[HEIGHT - 2][i] == 'O')
            tab[HEIGHT - 2][i] = ' ';
    }
    tab[HEIGHT - 2][x] = 'O';
}

void game_tick(char tab[HEIGHT][WIDTH], int x, int *points, int *gameSpeed, bool *timerOn, bool *immortalAvailable, bool *immortalActive)
{
    for (int y = HEIGHT - 2; y >= 1; y--)
    {
        for (int x = 1; x < WIDTH - 1; x++)
        {
            tab[y][x] = tab[y - 1][x];
        }
    }
    for (int x = 1; x < WIDTH - 1; x++)
    {
        tab[1][x] = ' ';
    }
    if (tab[HEIGHT - 2][x] == '#' && *immortalActive == false)
    {
        printf("Game over!\n");
        exit(0);
    }
    if (tab[HEIGHT - 2][x] == '+')
    {
        *points += 10;
    }
    if (tab[HEIGHT - 2][x] == '^')
    {
        *gameSpeed *= 0.8;
        *timerOn = true;
    }
    if (tab[HEIGHT - 2][x] == 'i')
    {
        *immortalAvailable = true;
    }
}

void setNonBlockingInput()
{
    struct termios newt;
    tcgetattr(STDIN_FILENO, &newt);
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);
}

void resetInputMode()
{
    struct termios oldt;
    tcgetattr(STDIN_FILENO, &oldt);
    oldt.c_lflag |= (ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
}

int main()
{
    char tab[HEIGHT][WIDTH];
    char user_input;
    int points = 0;
    int gameSpeed = BASE_SPEED;
    int SpeedBoostTimer = 0;
    int ImmortalTimer = 0;
    bool ImmortalTimerStatus = false;
    bool SpeedBoostTimerStatus = false;
    bool immortalActive = false;
    bool immortalAvailable = false;

    srand(time(NULL));
    setNonBlockingInput();

    for (int y = 0; y < HEIGHT; y++)
    {
        for (int x = 0; x < WIDTH; x++)
            tab[y][x] = ' ';
    }
    frame(tab);

    int playerX = WIDTH / 2;

    tab[HEIGHT - 2][WIDTH / 2] = 'O';

    while (1)
    {

        if (SpeedBoostTimerStatus == true)
            SpeedBoostTimer++;

        if (SpeedBoostTimer == 30)
        {
            gameSpeed = BASE_SPEED;
            SpeedBoostTimerStatus = false;
            SpeedBoostTimer = 0;
        }
        if (ImmortalTimerStatus == true)
            ImmortalTimer++;

        if (ImmortalTimer == 30)
        {
            immortalActive = false;
            ImmortalTimerStatus = false;
            ImmortalTimer = 0;
            immortalAvailable = false;
        }
        user_input = getch();
        system("clear");
        // printf("\033[H\033[J");
        printf("Punkty: %d", points++);
        if (immortalActive == true)
            printf("        Niesmiertelnosc : \033[0;32mAKTYWNA\033[0m \n");
        else
            printf("        Niesmiertelnosc : \033[0;31mNIEAKTYWNA\033[0m \n");
        if (user_input == 'a')
        {
            if (playerX == 1)
                continue;
            playerX--;
        }
        if (user_input == 'd')
        {
            if (playerX == WIDTH - 2)
                continue;
            playerX++;
        }
        if (user_input == 'i')
        {
            if (immortalAvailable == true)
            {
                immortalActive = true;
                ImmortalTimerStatus = true;
            }
        }
        if (user_input == 'p')
            pauseGame();

        // Spawn_obsticles(tab);
        // Spawn_bonus(tab);
        // Spawn_SpeedBoost(tab);
        spawn_item(tab, '#', 5);
        spawn_item(tab, '+', 4);
        spawn_item(tab, '^', 10);
        spawn_item(tab, 'i', 20);
        game_tick(tab, playerX, &points, &gameSpeed, &SpeedBoostTimerStatus, &immortalAvailable, &immortalActive);
        move_player(tab, playerX);
        display_tab(tab);

        usleep(gameSpeed);
    }
    resetInputMode();
    printf("Game Over\n");

    return 0;
}
