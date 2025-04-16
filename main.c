#include "main.h"
#include "images/alienBitmap.h"
#include "images/bulletBitmap.h"
#include "images/lose.h"
#include "images/rusherAlienBitmap.h"
#include "images/ship.h"
#include "images/start.h"

#include <stdio.h>
#include <stdlib.h>

Player player;
Alien aliens[MAX_ALIENS];
Bullet bullets[MAX_BULLETS];
Bullet alienBullets[MAX_ALIEN_BULLETS];

int aliensRemaining;
int frameCounter;
char scoreText[20];
int score;
int titleAnimationPos;
int titleAnimationDir;

int prevLives = 0;
int prevScore = 0;

void initializeGame(void);
void drawGame(void);
void updateGame(void);
void checkCollisions(void);
void drawTitleScreen(void);
void animateTitleScreen(void);
void drawLoseScreen(void);
void resetGame(void);
void firePlayerBullet(void);
void fireAlienBullet(int alienIndex);
int randChance(int chance);

int main(void)
{
    REG_DISPCNT = MODE3 | BG2_ENABLE;

    u32 previousButtons = BUTTONS;
    u32 currentButtons = BUTTONS;

    GameState state = START;

    titleAnimationPos = 0;
    titleAnimationDir = 1;
    frameCounter = 0;

    u8 endDrawn = 0;

    drawFullScreenImageDMA(start);

    while (1) {
        waitForVBlank();
        currentButtons = BUTTONS;

        if (KEY_JUST_PRESSED(BUTTON_SELECT, currentButtons, previousButtons)) {
            state = START;
            drawFullScreenImageDMA(start);
            titleAnimationPos = 0;
            titleAnimationDir = 1;
            frameCounter = 0;
        }

        switch (state) {
        case START:

            endDrawn = 0;
            if (frameCounter % 30 == 0)
                animateTitleScreen();

            if (KEY_JUST_PRESSED(BUTTON_START, currentButtons, previousButtons)) {
                state = PLAY;
                initializeGame();
                drawGame();
            }
            break;

        case PLAY:
            updateGame();

            // Check for button input]
            if (KEY_JUST_PRESSED(BUTTON_START, currentButtons, previousButtons)) {
                state = START;
                drawFullScreenImageDMA(start);
                titleAnimationPos = 0;
                titleAnimationDir = 1;
                frameCounter = 0;
            }
            if (KEY_DOWN(BUTTON_LEFT, currentButtons) && player.x > 0) {
                player.x -= PLAYER_SPEED;
            }
            if (KEY_DOWN(BUTTON_RIGHT, currentButtons) && player.x < WIDTH - SHIP_WIDTH) {
                player.x += PLAYER_SPEED;
            }
            if (KEY_JUST_PRESSED(BUTTON_A, currentButtons, previousButtons)) {
                if (player.bulletCooldown <= 0) {
                    firePlayerBullet();
                    player.bulletCooldown = BULLET_COOLDOWN;
                }
            }

            if (player.bulletCooldown > 0) {
                player.bulletCooldown--;
            }

            if (aliensRemaining <= 0) {
                state = WIN;
            }

            if (player.lives <= 0) {
                state = LOSE;
            }

            drawGame();
            break;

        case WIN:
            drawString(HEIGHT / 2 - 4, WIDTH / 2 - 50, "LEVEL COMPLETE!", WHITE);
            drawString(HEIGHT / 2 + 6, WIDTH / 2 - 80, "Press START to play again", WHITE);

            if (KEY_JUST_PRESSED(BUTTON_START, currentButtons, previousButtons)) {
                state = PLAY;
                initializeGame();
                drawGame();
            }
            break;

        case LOSE:
            if (!endDrawn) {
                endDrawn = 1;
                drawFullScreenImageDMA(lose);
                sprintf(scoreText, "FINAL SCORE: %d", score);
                drawString(HEIGHT / 2 + 20, WIDTH / 2 - 50, scoreText, WHITE);
                drawString(HEIGHT / 2 + 30, WIDTH / 2 - 80, "Press START to play again", WHITE);
            }

            // Press START to restart
            if (KEY_JUST_PRESSED(BUTTON_START, currentButtons, previousButtons)) {
                state = START;
                drawFullScreenImageDMA(start);
                titleAnimationPos = 0;
                titleAnimationDir = 1;
                frameCounter = 0;
            }
            break;
        }

        previousButtons = currentButtons;
        frameCounter++;
    }

    return 0;
}

void initializeGame(void)
{
    player.x = WIDTH / 2 - 8;
    player.y = HEIGHT - 25;
    player.lives = PLAYER_LIVES;
    player.bulletCooldown = 0;

    aliensRemaining = 0;
    for (int i = 0; i < ALIEN_ROWS; i++) {
        for (int j = 0; j < ALIEN_COLS; j++) {
            int index = i * ALIEN_COLS + j;
            aliens[index].x = 40 + j * 40;
            aliens[index].y = 30 + i * 30;
            aliens[index].dx = 1;
            aliens[index].dy = 0;
            aliens[index].alive = 1;
            aliens[index].type = NORMAL;
            aliens[index].rushCooldown = 0;
            aliensRemaining++;
        }
    }

    for (int i = 0; i < MAX_BULLETS; i++) {
        bullets[i].active = 0;
    }

    for (int i = 0; i < MAX_ALIEN_BULLETS; i++) {
        alienBullets[i].active = 0;
    }

    score = 0;

    fillScreenDMA(BLACK);
}

void drawGame(void)
{
    // fillScreenDMA(BLACK);
    if (prevLives != player.lives || prevScore != score) {
        fillScreenDMA(BLACK);
        prevLives = player.lives;
        prevScore = score;
    } else {
        drawRectDMA(15, 0, WIDTH, HEIGHT - 15, BLACK);
    }

    drawImageDMA(player.y, player.x, SHIP_WIDTH, SHIP_HEIGHT, ship);

    for (int i = 0; i < MAX_ALIENS; i++) {
        if (aliens[i].alive) {
            if (aliens[i].type == NORMAL) {
                drawImageDMA(aliens[i].y, aliens[i].x, ALIEN_WIDTH, ALIEN_HEIGHT, alienBitmap);
            } else {
                drawImageDMA(aliens[i].y, aliens[i].x, RUSHER_ALIEN_WIDTH, RUSHER_ALIEN_HEIGHT, rusherAlienBitmap);
            }
        }
    }

    for (int i = 0; i < MAX_BULLETS; i++) {
        if (bullets[i].active && bullets[i].y >= 15) {
            drawImageDMA(bullets[i].y, bullets[i].x, BULLETBITMAP_WIDTH, BULLETBITMAP_HEIGHT, bulletBitmap);
        }
    }

    for (int i = 0; i < MAX_ALIEN_BULLETS; i++) {
        if (alienBullets[i].active) {
            drawImageDMA(alienBullets[i].y, alienBullets[i].x, BULLETBITMAP_WIDTH, BULLETBITMAP_HEIGHT, bulletBitmap);
        }
    }

    char livesText[20];
    sprintf(livesText, "LIVES: %d", player.lives);
    drawString(5, 5, livesText, WHITE);

    sprintf(scoreText, "SCORE: %d", score);
    drawString(5, WIDTH - 80, scoreText, WHITE);
}

int abs(int x)
{
    return (x < 0) ? -x : x;
}

void updateGame(void)
{
    for (int i = 0; i < MAX_ALIENS; i++) {
        if (aliens[i].alive) {
            aliens[i].x += aliens[i].dx;
            aliens[i].y += aliens[i].dy;

            if ((aliens[i].x <= 0) || (aliens[i].x + ALIEN_WIDTH >= WIDTH)) {
                // Reverse direction for all aliens
                for (int j = 0; j < MAX_ALIENS; j++) {
                    if (aliens[j].alive) {
                        aliens[j].dx *= -1;
                        aliens[j].y += 5; // Move down when hitting the wall
                    }
                }
            }

            if (aliens[i].type == RUSHER) {
                aliens[i].dy = 2;

                if (aliens[i].y >= HEIGHT) {
                    aliens[i].y = 30;
                    aliens[i].type = NORMAL;
                    aliens[i].dy = 0;
                }
            } else {
                // Random chance to become a rusher
                if (randChance(RUSHER_CHANCE) && aliens[i].rushCooldown <= 0) {
                    aliens[i].type = RUSHER;
                    aliens[i].rushCooldown = 120;
                }
            }

            if (aliens[i].rushCooldown > 0) {
                aliens[i].rushCooldown--;
            }

            if (randChance(100) && aliens[i].type == NORMAL) {
                fireAlienBullet(i);
            }
        }
    }

    // player bullets
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (bullets[i].active) {
            bullets[i].y -= BULLET_SPEED;

            // Deactivate bullets that go off screen
            if (bullets[i].y < 0) {
                bullets[i].active = 0;
            }
        }
    }

    // Update alien bullets
    for (int i = 0; i < MAX_ALIEN_BULLETS; i++) {
        if (alienBullets[i].active) {
            alienBullets[i].y += ALIEN_BULLET_SPEED;

            // Deactivate bullets that go off screen
            if (alienBullets[i].y > HEIGHT) {
                alienBullets[i].active = 0;
            }
        }
    }

    checkCollisions();
}

void checkCollisions(void)
{
    // Check player bullets against aliens
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (bullets[i].active) {
            for (int j = 0; j < MAX_ALIENS; j++) {
                if (aliens[j].alive) {
                    if (bullets[i].x < aliens[j].x + ALIEN_WIDTH && bullets[i].x + BULLETBITMAP_WIDTH > aliens[j].x && bullets[i].y < aliens[j].y + ALIEN_HEIGHT && bullets[i].y + BULLETBITMAP_HEIGHT > aliens[j].y) {
                        // Collision detected

                        bullets[i].active = 0;
                        aliens[j].alive = 0;
                        aliensRemaining--;

                        // Add to score
                        if (aliens[j].type == RUSHER) {
                            score += 150;
                        } else {
                            score += 100;
                        }

                        break;
                    }
                }
            }
        }
    }

    // Check alien bullets against player
    for (int i = 0; i < MAX_ALIEN_BULLETS; i++) {
        if (alienBullets[i].active) {
            if (alienBullets[i].x < player.x + SHIP_WIDTH && alienBullets[i].x + BULLETBITMAP_WIDTH > player.x && alienBullets[i].y < player.y + SHIP_HEIGHT && alienBullets[i].y + BULLETBITMAP_HEIGHT > player.y) {

                // Collision detected
                alienBullets[i].active = 0;
                player.lives--;

                break;
            }
        }
    }

    // Check rushing aliens against player
    for (int j = 0; j < MAX_ALIENS; j++) {
        if (aliens[j].alive && aliens[j].type == RUSHER) {
            if (aliens[j].x < player.x + SHIP_WIDTH && aliens[j].x + ALIEN_WIDTH > player.x && aliens[j].y < player.y + SHIP_HEIGHT && aliens[j].y + ALIEN_HEIGHT > player.y) {

                // Collision detected
                aliens[j].alive = 0;
                aliens[j].type = NORMAL;
                aliensRemaining--;
                player.lives--;

                score += 50; // Still get some points for being hit

                break;
            }
        }
    }
}

void animateTitleScreen(void)
{
    drawFullScreenImageDMA(start);

    char titleText[] = "GALAGA";

    for (int i = 0; i < 6; i++) {
        if (i != titleAnimationPos) {
            drawChar(40, 85 + i * 12, titleText[i], WHITE);
        }
    }

    titleAnimationPos += titleAnimationDir;

    if (titleAnimationPos >= 6) {
        titleAnimationPos = 5;
        titleAnimationDir = -1;
    } else if (titleAnimationPos < 0) {
        titleAnimationPos = 0;
        titleAnimationDir = 1;
    }

    if ((frameCounter / 60) % 2 == 0) {
        drawString(100, 60, "Press START to Play", WHITE);
    }
}

void firePlayerBullet(void)
{
    // Find an inactive bullet
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (!bullets[i].active) {
            bullets[i].x = player.x + (SHIP_WIDTH / 2) - (BULLETBITMAP_WIDTH / 2);
            bullets[i].y = player.y - BULLETBITMAP_HEIGHT;
            bullets[i].active = 1;
            break;
        }
    }
}

void fireAlienBullet(int alienIndex)
{
    // Find an inactive bullet
    for (int i = 0; i < MAX_ALIEN_BULLETS; i++) {
        if (!alienBullets[i].active) {
            alienBullets[i].x = aliens[alienIndex].x + (ALIEN_WIDTH / 2) - (BULLETBITMAP_WIDTH / 2);
            alienBullets[i].y = aliens[alienIndex].y + ALIEN_HEIGHT;
            alienBullets[i].active = 1;
            break;
        }
    }
}

int randChance(int chance)
{
    // Return 1 with a 1 in 'chance' probability
    return (randint(0, chance) == 0);
}