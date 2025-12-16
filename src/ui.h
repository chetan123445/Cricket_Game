#ifndef UI_H
#define UI_H

#include "raylib.h"

void print_menu_box(const char *title, const char *options[], int num_options);
void clean_stdin();

// ICC theme colors used by selection UIs
#define ICC_BG        (Color){ 12, 28, 44, 255 }
#define ICC_BLUE      (Color){ 0, 92, 171, 255 }
#define ICC_GREEN     (Color){ 0, 153, 102, 255 }
#define ICC_YELLOW    (Color){ 255, 204, 0, 255 }
#define ICC_WHITE     (Color){ 245, 245, 245, 255 }
#define ICC_GRAY      (Color){ 160, 160, 160, 255 }
#define ICC_RED       (Color){ 200, 40, 40, 255 }

#endif // UI_H
