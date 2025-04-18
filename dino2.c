#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <conio.h>
#include <stdbool.h>

#define GROUND_Y 20
#define DINO_CHAR 'D'
#define OBSTACLE_CHAR '#'
#define JUMP_HEIGHT 5 
#define SCREEN_WIDTH 50
#define MAX_HIGH_SCORES 5
#define MAX_OBSTACLES 5  // Maximum number of obstacles on screen
#define OBSTACLE_GENERATION_RATE 20

typedef struct {
    int x, y;
    bool is_jumping;
    int jump_velocity;
} Dino;

// Stack for Jump History
typedef struct StackNode {
    int height;
    struct StackNode *next;
} StackNode;

typedef struct {
    StackNode *top;
} JumpStack;

typedef struct ObstacleNode {
    int x, y;
    struct ObstacleNode *next;
} ObstacleNode;

typedef struct {
    ObstacleNode *front, *rear;
    int size;  // Track the number of obstacles
} ObstacleQueue;

typedef struct HighScoreNode {
    int score;
    struct HighScoreNode *next;
} HighScoreNode;

void init_game(Dino *dino, ObstacleQueue *obstacles, JumpStack *jump_history);
void draw(Dino *dino, ObstacleQueue *obstacles, int score, HighScoreNode *high_scores);
void update_game(Dino *dino, ObstacleQueue *obstacles, JumpStack *jump_history, int *score, bool *game_over, int *frame_count);
void handle_input(Dino *dino, JumpStack *jump_history);
bool check_collision(Dino *dino, ObstacleQueue *obstacles);
void gotoxy(int x, int y);
void add_obstacle(ObstacleQueue *obstacles);
void remove_obstacle(ObstacleQueue *obstacles);
void push_jump(JumpStack *stack, int height);
int pop_jump(JumpStack *stack);
void add_high_score(HighScoreNode **head, int score);
void display_high_scores(HighScoreNode *head);
void free_obstacles(ObstacleQueue *obstacles);
void free_jump_history(JumpStack *jump_history);
void free_high_scores(HighScoreNode *head);

int main() {
    Dino dino;
    ObstacleQueue obstacles = {NULL, NULL, 0};  // Initialize queue with size 0
    JumpStack jump_history = {NULL};
    HighScoreNode *high_scores = NULL;
    int score;
    bool running = true;

    while (running) {
        bool game_over = false;
        score = 0;
        int frame_count = 0;  // Frame counter to control obstacle generation rate

        init_game(&dino, &obstacles, &jump_history);

        while (!game_over) {
            if (_kbhit()) {
                handle_input(&dino, &jump_history);
            }
            update_game(&dino, &obstacles, &jump_history, &score, &game_over, &frame_count);
            draw(&dino, &obstacles, score, high_scores);

            Sleep(50);
        }

        add_high_score(&high_scores, score);

        gotoxy(0, GROUND_Y + 3);
        printf("Game Over! Final Score: %d\n", score);
        display_high_scores(high_scores);
        printf("Press 'r' to play again or 'q' to quit.\n");

        char choice;
        do {
            choice = _getch();
        } while (choice != 'r' && choice != 'q');

        if (choice == 'q') {
            running = false;
        }

        free_obstacles(&obstacles);
        free_jump_history(&jump_history);
    }

    printf("Thanks for playing! Final High Scores:\n");
    display_high_scores(high_scores);
    free_high_scores(high_scores);
    return 0;
}

void init_game(Dino *dino, ObstacleQueue *obstacles, JumpStack *jump_history) {
    dino->x = 5;
    dino->y = GROUND_Y;
    dino->is_jumping = false;
    dino->jump_velocity = 0;

    add_obstacle(obstacles);  // Initial obstacle
}

void draw(Dino *dino, ObstacleQueue *obstacles, int score, HighScoreNode *high_scores) {
    system("cls");

    gotoxy(dino->x, dino->y);
    printf("%c", DINO_CHAR);

    ObstacleNode *current = obstacles->front;
    while (current) {
        gotoxy(current->x, current->y);
        printf("%c", OBSTACLE_CHAR);
        current = current->next;
    }

    gotoxy(0, GROUND_Y + 1);
    for (int i = 0; i < SCREEN_WIDTH; i++) {
        printf("_");
    }

    gotoxy(0, 0);
    printf("Score: %d", score);
}

void update_game(Dino *dino, ObstacleQueue *obstacles, JumpStack *jump_history, int *score, bool *game_over, int *frame_count) {
    if (dino->is_jumping) {
        dino->y += dino->jump_velocity;
        dino->jump_velocity += 1;

        if (dino->y >= GROUND_Y) {
            dino->y = GROUND_Y;
            dino->is_jumping = false;
        }
    }

    if (obstacles->front && obstacles->front->x < 0) {
        remove_obstacle(obstacles);
        *score += 1;
    }

    ObstacleNode *current = obstacles->front;
    while (current) {
        current->x -= 1;
        current = current->next;
    }

    (*frame_count)++;
    if (*frame_count >= OBSTACLE_GENERATION_RATE && obstacles->size < MAX_OBSTACLES) {
        add_obstacle(obstacles);
        *frame_count = 0;
    }

    if (check_collision(dino, obstacles)) {
        *game_over = true;
    }
}

void handle_input(Dino *dino, JumpStack *jump_history) {
    char key = _getch();
    if (key == ' ' && !dino->is_jumping) {
        dino->is_jumping = true;
        dino->jump_velocity = -2 * JUMP_HEIGHT;
        push_jump(jump_history, dino->y);
    }
}

bool check_collision(Dino *dino, ObstacleQueue *obstacles) {
    ObstacleNode *current = obstacles->front;
    return current && dino->x == current->x && dino->y == current->y;
}

void gotoxy(int x, int y) {
    COORD coord;
    coord.X = x;
    coord.Y = y;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

void add_obstacle(ObstacleQueue *obstacles) {
    ObstacleNode *new_obstacle = malloc(sizeof(ObstacleNode));
    new_obstacle->x = SCREEN_WIDTH;
    new_obstacle->y = GROUND_Y;
    new_obstacle->next = NULL;

    if (obstacles->rear) {
        obstacles->rear->next = new_obstacle;
    } else {
        obstacles->front = new_obstacle;
    }
    obstacles->rear = new_obstacle;
    obstacles->size++;  // Increase queue size
}

void remove_obstacle(ObstacleQueue *obstacles) {
    if (obstacles->front) {
        ObstacleNode *temp = obstacles->front;
        obstacles->front = obstacles->front->next;
        if (!obstacles->front) {
            obstacles->rear = NULL;
        }
        free(temp);
        obstacles->size--;  // Decrease queue size
    }
}

void push_jump(JumpStack *stack, int height) {
    StackNode *new_node = malloc(sizeof(StackNode));
    new_node->height = height;
    new_node->next = stack->top;
    stack->top = new_node;
}

int pop_jump(JumpStack *stack) {
    if (!stack->top) return -1;
    StackNode *temp = stack->top;
    int height = temp->height;
    stack->top = stack->top->next;
    free(temp);
    return height;
}

void add_high_score(HighScoreNode **head, int score) {
    HighScoreNode *new_node = malloc(sizeof(HighScoreNode));
    new_node->score = score;
    new_node->next = NULL;

    if (!*head || score > (*head)->score) {
        new_node->next = *head;
        *head = new_node;
    } else {
        HighScoreNode *current = *head;
        while (current->next && current->next->score > score) {
            current = current->next;
        }
        new_node->next = current->next;
        current->next = new_node;
    }

    int count = 1;
    HighScoreNode *temp = *head;
    while (temp->next && count < MAX_HIGH_SCORES) {
        temp = temp->next;
        count++;
    }

    if (temp->next) {
        HighScoreNode *to_delete = temp->next;
        temp->next = NULL;
        free_high_scores(to_delete);
    }
}

void display_high_scores(HighScoreNode *head) {
    printf("High Scores:\n");
    int rank = 1;
    while (head) {
        printf("%d. %d\n", rank++, head->score);
        head = head->next;
    }
}

void free_obstacles(ObstacleQueue *obstacles) {
    while (obstacles->front) {
        remove_obstacle(obstacles);
    }
}

void free_jump_history(JumpStack *jump_history) {
    while (jump_history->top) {
        pop_jump(jump_history);
    }
}

void free_high_scores(HighScoreNode *head) {
    HighScoreNode *current;
    while ((current = head) != NULL) {
        head = head->next;
        free(current);
    }
}
