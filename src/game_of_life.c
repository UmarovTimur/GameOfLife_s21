#include <malloc.h>
#include <stdio.h>
#include <termios.h>
#include <unistd.h>

// ------------------ Создание и работа с полем игры ---------------------

int **allocated_matrix(const int rows, const int cols);
int create_world(int **matrix, const int height, const int width);
void free_matrix(int **matrix, const int rows);

// ------------------ Рендеры мира ---------------------------------------

void loop(int **matrix, int **matrix2, int height, int width, int *speed_of_game);
void render_world(int **matrix2, int height, int width);
int count_neighbours(int **matrix, int height, int width, int x, int y);
int update_matrix(int **matrix1, int **matrix2, int height, int width, int count);

// ------------------ Проверки и рабочие функции -------------------------

int check_matrix(int **world, int w, int h);
void clean(void);
int decision(int element, int count);
void copy_matrix(int **matrix1, int **matrix2, int height, int width);
void change_speed(int *speed, char input);


// ------------------ Termios --------------------------------------------

void initTermios(struct termios *old, struct termios *raw, int echo);
void resetTermios(const struct termios *old);

// -----------------------------------------------------------------------
// Make change speed option

int main(void) {
    const int height = 25;
    const int width = 80;

    int **world1 = allocated_matrix(height, width);
    int **world2 = allocated_matrix(height, width);
    int speed_of_game = 500;

    if (world1 == NULL || world2 == NULL) {
        printf("Error with allocating memory\n");
        return 1;
    }

    // ------------------ Проверка мода игры ---------------------

    if (create_world(world1, height, width)) {
        printf("n/a");
        free_matrix(world1, height);
        free_matrix(world2, height);
        return 1;
    }

    // --------------------- Запуск игры -------------------------

    loop(world1, world2, height, width, &speed_of_game);

    free_matrix(world1, height);
    free_matrix(world2, height);
    return 0;
}

// -----------------------------------------------------------------------
void initTermios(struct termios *old, struct termios *raw, int echo) {
    tcgetattr(STDIN_FILENO, old);  // Save current terminal settings

    *raw = *old;     // Copy the settings to modify them
    cfmakeraw(raw);  // Make raw mode: disable canonical mode, echo, signals

    if (echo) {
        raw->c_lflag |= ECHO;  // Enable echo if specified
    } else {
        raw->c_lflag &= ~ECHO;  // Disable echo otherwise
    }

    tcsetattr(STDIN_FILENO, TCSANOW, raw);  // Apply raw settings immediately

    write(STDOUT_FILENO, "\e[?47h", 6);  // Enable alternate screen buffer
    write(STDOUT_FILENO, "\e[?9h", 5);   // Enable mouse input
}

void resetTermios(const struct termios *old) {
    write(STDOUT_FILENO, "\e[?9l", 5);
    write(STDOUT_FILENO, "\e[?47l", 6);
    tcsetattr(STDIN_FILENO, TCSANOW, old);
}

// -----------------------------------------------------------------------

void loop(int **matrix1, int **matrix2, int height, int width, int *speed_of_game) {
    struct termios original, raw;

    int stop = 1;

    initTermios(&original, &raw, 0);

    while (stop) {
        int count = 0;

        usleep(*speed_of_game * 1000);
        render_world(matrix1, height, width);

        char input = getchar();

        if (read(STDIN_FILENO, &input, 1) == 1) {
            change_speed(speed_of_game, input);
        }

        stop = update_matrix(matrix1, matrix2, height, width, count);

        copy_matrix(matrix1, matrix2, height, width);
    }

    resetTermios(&original);
}

int create_world(int **matrix, const int height, const int width) {
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            if (scanf("%d", &matrix[i][j]) != 1) {
                return 1;
            }
        }
    }
    return 0;
}

void render_world(int **matrix, const int height, const int width) {
    clean();
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            printf("%c%c", matrix[i][j] == 0 ? '`' : 'E', j == width - 1 ? '\n' : ' ');
        }
    }
}

void free_matrix(int **matrix, const int rows) {
    for (int i = 0; i < rows; i++) {
        free(matrix[i]);
    }
    free(matrix);
}

int **allocated_matrix(const int w_height, const int w_width) {
    int **matrix = malloc(w_width * sizeof(int *));

    if (matrix == NULL) {
        free(matrix);
        return NULL;
    }

    for (int i = 0; i < w_height; i++) {
        matrix[i] = malloc(w_width * sizeof(int));
        if (matrix[i] == NULL) {
            free_matrix(matrix, i);
        }
        for (int j = 0; j < w_width; j++) matrix[i][j] = 0;
    }

    return matrix;
}

void clean(void) { printf("\033[H\033[J"); }

int count_neighbours(int **matrix, int height, int width, int x, int y) {
    int count = 0;

    const int dx[8] = {-1, 0, 1, 0, -1, -1, 1, 1};
    const int dy[8] = {0, -1, 0, 1, -1, 1, -1, 1};

    for (int i = 0; i < 8; i++) {
        int new_x = (x + dx[i] + width) % width;
        int new_y = (y + dy[i] + height) % height;

        if (matrix[new_y][new_x]) count++;
    }
    return count;
}

int update_matrix(int **matrix1, int **matrix2, int height, int width, int count) {
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            matrix2[i][j] = decision(matrix1[i][j], count_neighbours(matrix1, height, width, j, i));
            count += matrix1[i][j] == matrix2[i][j];
        }
    }
    return count == 0 ? 0 : 1;
}

int decision(int element, int count) {
    if (!element && count == 3)
        return 1;
    else if ((element && count < 2) || count > 3)
        return 0;
    return element;
}

void copy_matrix(int **to, int **from, int height, int width) {
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            to[i][j] = from[i][j];
        }
    }
}

void change_speed(int *speed, char input) {
    if (input == '+')
        *speed = *speed / 2;
    else if (input == '-')
        *speed = *speed * 2;
}
