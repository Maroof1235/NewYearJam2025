#include "raylib.h"
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define MAX_OBJECTS 50
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define PLAYER_SIZE 40
#define OBJECT_SIZE 30
#define GAME_DURATION 30.0f

typedef struct {
    Vector2 position;
    float speed;
    bool active;
    Color color;
} FallingObject;

typedef struct {
    Vector2 position;
    float speed;
} Player;

typedef enum {
    MENU,
    PLAYING,
    GAME_OVER,
    WIN
} GameState;

int main(void) {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "30 Second Dodge");
    SetTargetFPS(60);
    
    srand(time(NULL));
    
    Player player = {0};
    FallingObject objects[MAX_OBJECTS] = {0};
    GameState gameState = MENU;
    float gameTimer = 0.0f;
    int score = 0;
    float spawnTimer = 0.0f;
    float spawnRate = 1.0f;
    
    while (!WindowShouldClose()) {
        float deltaTime = GetFrameTime();
        
        switch (gameState) {
            case MENU:
                if (IsKeyPressed(KEY_SPACE)) {
                    // Reset game
                    player.position = (Vector2){SCREEN_WIDTH / 2 - PLAYER_SIZE / 2, SCREEN_HEIGHT - 80};
                    player.speed = 320.0f;
                    gameTimer = 0.0f;
                    score = 0;
                    spawnTimer = 0.0f;
                    spawnRate = 1.0f;
                    
                    for (int i = 0; i < MAX_OBJECTS; i++) {
                        objects[i].active = false;
                    }
                    
                    gameState = PLAYING;
                }
                break;
                
            case PLAYING:
                gameTimer += deltaTime;
                
                // Check if player won
                if (gameTimer >= GAME_DURATION) {
                    gameState = WIN;
                    break;
                }
                
                // Increase difficulty over time
                spawnRate = 1.0f - (gameTimer / GAME_DURATION) * 0.7f;
                
                // Player movement
                if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)) {
                    player.position.x -= player.speed * deltaTime;
                }
                if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) {
                    player.position.x += player.speed * deltaTime;
                }
                
                // Keep player in bounds
                if (player.position.x < 0) player.position.x = 0;
                if (player.position.x > SCREEN_WIDTH - PLAYER_SIZE) {
                    player.position.x = SCREEN_WIDTH - PLAYER_SIZE;
                }
                
                // Spawn falling objects
                spawnTimer += deltaTime;
                if (spawnTimer >= spawnRate) {
                    spawnTimer = 0.0f;
                    
                    // Find inactive object slot
                    for (int i = 0; i < MAX_OBJECTS; i++) {
                        if (!objects[i].active) {
                            objects[i].active = true;
                            objects[i].position.x = rand() % (SCREEN_WIDTH - OBJECT_SIZE);
                            objects[i].position.y = -OBJECT_SIZE;
                            objects[i].speed = 180.0f + (gameTimer / GAME_DURATION) * 220.0f;
                            
                            // Random colors
                            int colorChoice = rand() % 4;
                            if (colorChoice == 0) objects[i].color = (Color){255, 80, 80, 255};      // Red
                            else if (colorChoice == 1) objects[i].color = (Color){255, 180, 60, 255}; // Orange
                            else if (colorChoice == 2) objects[i].color = (Color){200, 100, 255, 255}; // Purple
                            else objects[i].color = (Color){255, 220, 80, 255};                       // Yellow
                            
                            break;
                        }
                    }
                }
                
                // Update falling objects
                for (int i = 0; i < MAX_OBJECTS; i++) {
                    if (objects[i].active) {
                        objects[i].position.y += objects[i].speed * deltaTime;
                        
                        // Check collision with player
                        Rectangle objectRect = {objects[i].position.x, objects[i].position.y, 
                                               OBJECT_SIZE, OBJECT_SIZE};
                        Rectangle playerRect = {player.position.x, player.position.y, 
                                               PLAYER_SIZE, PLAYER_SIZE};
                        
                        if (CheckCollisionRecs(objectRect, playerRect)) {
                            gameState = GAME_OVER;
                        }
                        
                        // Deactivate if off screen
                        if (objects[i].position.y > SCREEN_HEIGHT) {
                            objects[i].active = false;
                            score++;
                        }
                    }
                }
                break;
                
            case GAME_OVER:
            case WIN:
                if (IsKeyPressed(KEY_SPACE)) {
                    gameState = MENU;
                }
                break;
        }
        
        // Drawing
        BeginDrawing();
        
        // Nice gradient background
        DrawRectangleGradientV(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 
                              (Color){25, 30, 50, 255}, 
                              (Color){50, 40, 70, 255});
        
        switch (gameState) {
            case MENU:
                DrawText("30 SECOND", SCREEN_WIDTH / 2 - 180, SCREEN_HEIGHT / 2 - 110, 50, WHITE);
                DrawText("DODGE", SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2 - 50, 50, (Color){100, 200, 255, 255});
                DrawText("Press SPACE to start", SCREEN_WIDTH / 2 - 150, SCREEN_HEIGHT / 2 + 30, 28, LIGHTGRAY);
                DrawText("Arrow Keys / A & D to move", SCREEN_WIDTH / 2 - 170, SCREEN_HEIGHT / 2 + 70, 20, GRAY);
                break;
                
            case PLAYING:
                // Draw player with slight shadow
                DrawRectangle(player.position.x + 3, player.position.y + 3, PLAYER_SIZE, PLAYER_SIZE, 
                             (Color){0, 0, 0, 80});
                DrawRectangleV(player.position, (Vector2){PLAYER_SIZE, PLAYER_SIZE}, 
                              (Color){80, 180, 255, 255});
                DrawRectangleLinesEx((Rectangle){player.position.x, player.position.y, PLAYER_SIZE, PLAYER_SIZE}, 
                                    2, WHITE);
                
                // Draw falling objects with shadows
                for (int i = 0; i < MAX_OBJECTS; i++) {
                    if (objects[i].active) {
                        DrawRectangle(objects[i].position.x + 2, objects[i].position.y + 2, 
                                     OBJECT_SIZE, OBJECT_SIZE, (Color){0, 0, 0, 80});
                        DrawRectangleV(objects[i].position, 
                                      (Vector2){OBJECT_SIZE, OBJECT_SIZE}, 
                                      objects[i].color);
                    }
                }
                
                // UI
                float timeLeft = GAME_DURATION - gameTimer;
                Color timeColor = timeLeft <= 10 ? (Color){255, 100, 100, 255} : WHITE;
                DrawText(TextFormat("TIME: %.1f", timeLeft), 10, 10, 32, timeColor);
                DrawText(TextFormat("Score: %d", score), 10, 50, 26, (Color){255, 220, 100, 255});
                
                // Timer bar with border
                float progress = gameTimer / GAME_DURATION;
                DrawRectangle(0, SCREEN_HEIGHT - 12, SCREEN_WIDTH, 12, (Color){30, 30, 30, 200});
                DrawRectangle(0, SCREEN_HEIGHT - 12, SCREEN_WIDTH * progress, 12, 
                             (Color){100, 220, 120, 255});
                break;
                
            case GAME_OVER:
                DrawText("GAME OVER", SCREEN_WIDTH / 2 - 170, SCREEN_HEIGHT / 2 - 80, 55, 
                        (Color){255, 100, 100, 255});
                DrawText(TextFormat("Score: %d objects dodged", score), SCREEN_WIDTH / 2 - 160, 
                        SCREEN_HEIGHT / 2 + 10, 28, WHITE);
                DrawText(TextFormat("Time survived: %.1fs", gameTimer), SCREEN_WIDTH / 2 - 140, 
                        SCREEN_HEIGHT / 2 + 50, 24, LIGHTGRAY);
                DrawText("Press SPACE to retry", SCREEN_WIDTH / 2 - 130, SCREEN_HEIGHT / 2 + 110, 24, GRAY);
                break;
                
            case WIN:
                DrawText("YOU WIN!", SCREEN_WIDTH / 2 - 140, SCREEN_HEIGHT / 2 - 80, 60, 
                        (Color){255, 220, 100, 255});
                DrawText(TextFormat("Final Score: %d", score), SCREEN_WIDTH / 2 - 120, 
                        SCREEN_HEIGHT / 2 + 10, 30, WHITE);
                DrawText("You survived 30 seconds!", SCREEN_WIDTH / 2 - 150, 
                        SCREEN_HEIGHT / 2 + 50, 24, LIGHTGRAY);
                DrawText("Press SPACE to play again", SCREEN_WIDTH / 2 - 140, SCREEN_HEIGHT / 2 + 110, 24, GRAY);
                break;
        }
        
        EndDrawing();
    }
    
    CloseWindow();
    return 0;
}