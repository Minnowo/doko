


#include <raylib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ui.h"
#include "config.h"
#include "darray.h"
#include "doko.h"


int hasInit = 0;

char* imageBufPath;
Texture2D imageBuf;
Texture2D backgroundBuf;

Color pixelGridColor;

void ui_init() {

    InitWindow(START_WIDTH, START_HEIGHT, WINDOW_TITLE);
    SetWindowMinSize(MIN_WINDOW_WIDTH, MIN_WINDOW_HEIGHT);

    SetWindowState(FLAG_WINDOW_RESIZABLE);
    SetTargetFPS(WINDOW_FPS);
    SetExitKey(RAYLIB_QUIT_KEY);

    backgroundBuf = ui_loadBackgroundTile(BACKGROUND_TILE_W, BACKGROUND_TILE_H,
                                       (Color)BACKGROUND_TILE_COLOR_A_RGBA,
                                       (Color)BACKGROUND_TILE_COLOR_B_RGBA);

    pixelGridColor = (Color)PIXEL_GRID_COLOR_RGBA;

    imageBuf.id = 0;
    imageBufPath = NULL;
    hasInit = 1;
}

void ui_deinit() {

    if(!hasInit) {
        exit(1);
        return;
    }

    UnloadTexture(backgroundBuf);

    if (imageBuf.id > 0) {
        UnloadTexture(imageBuf);
    }

    CloseWindow();

    hasInit = 0;
}


Texture2D ui_loadBackgroundTile(size_t w, size_t h, Color a, Color b) {

    Image image = GenImageColor(w, h, a);
    ImageDrawRectangle(&image, 0, 0, w / 2, h / 2, b);
    ImageDrawRectangle(&image, w / 2, h / 2, w / 2, h / 2, b);

    Texture2D t = LoadTextureFromImage(image);

    UnloadImage(image);

    return t;
}

void ui_renderTextOnInfoBar(const char* text) {

    int sw = GetScreenWidth() ;
    int sh = ImageViewHeight;

    int fontSize = INFO_BAR_FONT_SIZE;

    int textSize;

    do {
        textSize = MeasureText(text, fontSize);
    } while (textSize > sw - INFO_BAR_LEFT_MARGIN && --fontSize);

    DrawRectangle(0, sh, sw,  INFO_BAR_HEIGHT, BLACK);

    DrawText(text, INFO_BAR_LEFT_MARGIN, sh,  fontSize, WHITE);
}

void ui_renderBackground() {

    DrawTextureRec(backgroundBuf,
                   (Rectangle){0, 0, GetScreenWidth(), GetScreenHeight()},
                   (Vector2){0, 0}, WHITE);
}

void ui_renderImage(doko_image_t* image) {

    if(image->status == IMAGE_STATUS_FAILED) {
        return;
    }

    if (imageBufPath != image->path) {

        if (image->status == IMAGE_STATUS_NOT_LOADED) {
            if (!doko_loadImage(image)) {
                return;
            }
        }

        Texture2D nimageBuf = LoadTextureFromImage(image->rayim);

        if(nimageBuf.id == 0) {
            return;
        }

        if(imageBuf.id > 0) {
            UnloadTexture(imageBuf);
        }

        imageBufPath = image->path;
        imageBuf = nimageBuf;
    }
    else if (image->rebuildBuff) {

        image->rebuildBuff = 0;
        UpdateTexture(imageBuf,image->rayim.data);
    }

    DrawTextureEx(imageBuf, image->dstPos, image->rotation, image->scale,
                  WHITE);
}

void ui_renderInfoBar(doko_image_t *image) {
    ui_renderTextOnInfoBar(
        TextFormat("%0.0f x %0.0f   %s", image->srcRect.width,
                   image->srcRect.height, image->path + image->nameOffset));
}

void ui_renderPixelGrid(doko_image_t* image) {

    if (image->scale < SHOW_PIXEL_GRID_SCALE_THRESHOLD) {
        return;
    }

    float x = image->dstPos.y;
    float y = image->dstPos.x;
    float w = ImageViewWidth;
    float h = ImageViewHeight;

    for (float i = x, j = y; i <= h || j <= w;
         i += image->scale, j += image->scale) {
        DrawLine(0, i, w, i, pixelGridColor);
        DrawLine(j, 0, j, h, pixelGridColor);
    }

    for (float i = x, j = y; i > 0 || j > 0;
         i -= image->scale, j -= image->scale) {
        DrawLine(0, i, w, i, pixelGridColor);
        DrawLine(j, 0, j, h, pixelGridColor);
    }
}


void ui_renderFileList(doko_control_t* ctrl) {

    int sw = GetScreenWidth() ;
    int sh = GetScreenHeight();

    #define FZ FILE_LIST_FONT_SIZE

    int startY = sh - ctrl->image_files.size * FZ;

    DARRAY_FOR_EACH_I(ctrl->image_files, i) {

        doko_image_t *im = ctrl->image_files.buffer + i;

        int y = startY + FZ * i;

        if (ctrl->selected_image == NULL || i != ctrl->selected_index) {
            DrawRectangle(0, y, sw, FZ, BLACK);
        } else {
            DrawRectangle(0, y, sw, FZ, GREEN);
        }

        DrawText(im->path + im->nameOffset, FILE_LIST_LEFT_MARGIN, y, FZ,
                 WHITE);
    }
}
