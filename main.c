#include <math.h>
#include <raylib.h>
#include <raymath.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

Color colors[] = {RED, GREEN, BLUE, YELLOW, PURPLE, SKYBLUE, PINK, BROWN};
#define COLORS_COUNT (sizeof(colors) / sizeof(colors[0]))
#define START_TEXT "Press any button to start"
#define WIDTH 1280
#define HEIGHT 720
#define BUTTON_COUNT 4
#define RADIUS ((int)(HEIGHT / (BUTTON_COUNT + 3)))
#define RECTANGLE_WIDTH (WIDTH / BUTTON_COUNT)
#define DARKNESS -0.65
#define SHOW_TIME 0.5
#define WAIT_TIME 0.2
#define INITIAL_COUNT 8

typedef enum {
  GAME_STATE_START,
  GAME_STATE_PLAYBACK,
  GAME_STATE_INPUT,
} GameState;

typedef struct {
  int count;
  int *data;
} Pattern;

Pattern pattern_new() {
  int *data = malloc(INITIAL_COUNT * sizeof(int));
  for (int i = 0; i < INITIAL_COUNT; i++) {
    data[i] = rand() % BUTTON_COUNT;
  }
  return (Pattern){INITIAL_COUNT, data};
}

void pattern_randomize(Pattern *p) {
  for (int i = 0; i < p->count; i++) {
    p->data[i] = rand() % BUTTON_COUNT;
  }
}

int pattern_get(Pattern *p, int index) {
  if (index >= p->count) {
    int new_count = p->count * 2;
    p->data = realloc(p->data, new_count * sizeof(int));
    for (int i = p->count; i < new_count; i++) {
      p->data[i] = rand() % BUTTON_COUNT;
    }
    p->count = new_count;
  }
  return p->data[index];
}

// typedef struct {
//   GameState game_state;
//   int level;
//   double time;
//   int input_index;
// } State;

Vector2 index_to_vec(int rec_i) {
  int rec_start = rec_i * RECTANGLE_WIDTH;
  int rec_end = (rec_i + 1) * RECTANGLE_WIDTH;
  int center_x = ((rec_start + rec_end) / 2);
  int center_y = HEIGHT / 2;
  return (Vector2){center_x, center_y};
}

int main(void) {
  srand(time(NULL));
  SetTraceLogLevel(LOG_WARNING);
  SetConfigFlags(FLAG_MSAA_4X_HINT);
  SetTargetFPS(60);
  InitWindow(WIDTH, HEIGHT, "Remember");

  Pattern pattern = pattern_new();
  GameState game_state = GAME_STATE_START;
  int level = 1;
  double time = 0;
  int player_choice = 0;
  double lightining[BUTTON_COUNT] = {0};

  for (int i = 0; i < BUTTON_COUNT; i++) {
    lightining[i] = SHOW_TIME + WAIT_TIME;
  }
  pattern_randomize(&pattern);

  while (!WindowShouldClose()) {
    BeginDrawing();
    ClearBackground(WHITE);

    for (int i = 0; i < BUTTON_COUNT; i++) {
      DrawCircleV(index_to_vec(i), RADIUS, ColorBrightness(colors[i % COLORS_COUNT], DARKNESS));
    }

    switch (game_state) {
    case GAME_STATE_START: {
      DrawText(START_TEXT, (WIDTH - MeasureText(START_TEXT, 20)) / 2, HEIGHT / 8, 20, GRAY);
      if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        game_state = GAME_STATE_PLAYBACK;
      }
      break;
    }
    case GAME_STATE_PLAYBACK: {
      if (level * (SHOW_TIME + WAIT_TIME) < time) {
        game_state = GAME_STATE_INPUT;
        time = 0;
        break;
      }
      if (fmod(time, SHOW_TIME + WAIT_TIME) < SHOW_TIME) {
        int i = pattern_get(&pattern, time / (SHOW_TIME + WAIT_TIME));
        DrawCircleV(index_to_vec(i), RADIUS, ColorBrightness(colors[i % COLORS_COUNT], 0));
      }
      time += GetFrameTime();
      break;
    }
    case GAME_STATE_INPUT: {
      bool animating = false;
      for (int i = 0; i < BUTTON_COUNT; i++) {
        if (lightining[i] < SHOW_TIME + WAIT_TIME) {
          animating = true;
          if (lightining[i] < SHOW_TIME) {
            DrawCircleV(index_to_vec(i), RADIUS, ColorBrightness(colors[i % COLORS_COUNT], 0));
          }
          lightining[i] += GetFrameTime();
        }
      }
      if (!animating && player_choice == level) {
        if (time > WAIT_TIME * 2) {
          game_state = GAME_STATE_PLAYBACK;
          player_choice = 0;
          time = 0;
          level++;
        } else {
          time += GetFrameTime();
        }
        break;
      }
      if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && player_choice < level) {
        int i = GetMouseX() / RECTANGLE_WIDTH;
        if (!CheckCollisionPointCircle((Vector2){GetMouseX(), GetMouseY()}, index_to_vec(i), RADIUS)) {
          break;
        }
        if (pattern_get(&pattern, player_choice) == i) {
          lightining[i] = 0;
          player_choice++;
        } else {
          game_state = GAME_STATE_START;
          level = 1;
          player_choice = 0;
          for (int i = 0; i < BUTTON_COUNT; i++) {
            lightining[i] = SHOW_TIME + WAIT_TIME;
          }
          pattern_randomize(&pattern);
        }
      }
      break;
    }
    }

    EndDrawing();
  }
  free(pattern.data);
  CloseWindow();
  return 0;
}
