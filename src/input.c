
#include "input.h"
#include "config.h"
#include "doko.h"
#include <raylib.h>
#include <stdio.h>
#include <stdlib.h>

#define _ZERO_SIZE_WARN(c)                                                     \
    if ((c)->image_files.size == 0) {                                          \
        (c)->selected_image = NULL;                                            \
        L_W("There are 0 images!");                                            \
        return;                                                                \
    }

#define _NO_IMAGE_WARN(c)                                                      \
    if ((c)->selected_image == NULL) {                                         \
        L_W("There is no image!");                                             \
        return;                                                                \
    }                                                                          \
    if ((c)->selected_image->status != IMAGE_STATUS_LOADED) {                  \
        return;                                                                \
    }

#define I_X(i) (i)->selected_image->dstPos.x
#define I_Y(i) (i)->selected_image->dstPos.y
#define I_WIDTH(i) (i)->selected_image->srcRect.width
#define I_HEIGHT(i) (i)->selected_image->srcRect.height
#define I_SCALE(i) (i)->selected_image->scale

void kb_Zoom_In_Center_Image(doko_control_t* ctrl) {

    _NO_IMAGE_WARN(ctrl);

    doko_zoomImageOnPointFromClosest(
        ctrl->selected_image, true, GetScreenWidth() / 2, GetScreenHeight() / 2
    );
}

void kb_Zoom_Out_Center_Image(doko_control_t* ctrl) {

    _NO_IMAGE_WARN(ctrl);

    doko_zoomImageOnPointFromClosest(
        ctrl->selected_image, false, GetScreenWidth() / 2, GetScreenHeight() / 2
    );
}

void kb_Zoom_In_Mouse_Position(doko_control_t* ctrl) {

    _NO_IMAGE_WARN(ctrl);

    doko_zoomImageOnPointFromClosest(
        ctrl->selected_image, true, GetMouseX(), GetMouseY()
    );
}
void kb_Zoom_Out_Mouse_Position(doko_control_t* ctrl) {

    _NO_IMAGE_WARN(ctrl);

    doko_zoomImageOnPointFromClosest(
        ctrl->selected_image, false, GetMouseX(), GetMouseY()
    );
}

void kb_Next_Image_By_N(doko_control_t* ctrl, int by) {

    _ZERO_SIZE_WARN(ctrl);

    _NO_IMAGE_WARN(ctrl);

    // stop at the start when wrapping
    if (ctrl->selected_index == ctrl->image_files.size - 1) {

        set_image(ctrl, 0);
    }

    // stop at the end when jumping
    else if (by > 1 && (ctrl->selected_index + by) >= ctrl->image_files.size) {

        set_image(ctrl, ctrl->image_files.size - 1);
    }

    // otherwise move however
    else {

        set_image(ctrl, (ctrl->selected_index + by) % ctrl->image_files.size);
    }
}

void kb_Prev_Image_N(doko_control_t* ctrl, int by) {

    _ZERO_SIZE_WARN(ctrl);

    _NO_IMAGE_WARN(ctrl);

    // stop at the end when wrapping
    if (ctrl->selected_index == 0) {

        set_image(ctrl, ctrl->image_files.size - 1);
    }

    // stop at the start when jumping many
    else if (by > 1 && (ctrl->selected_index - by) <= 0) {

        set_image(ctrl, 0);
    }

    // otherwise move however
    else {

        set_image(
            ctrl, (ctrl->selected_index + ctrl->image_files.size - by) %
                      ctrl->image_files.size
        );
    }
}

void kb_Next_Image(doko_control_t* ctrl) {

    kb_Next_Image_By_N(ctrl, 1);
}

void kb_Next_Image_By_10(doko_control_t* ctrl) {

    kb_Next_Image_By_N(ctrl, 10);
}

void kb_Prev_Image(doko_control_t* ctrl) {

    kb_Prev_Image_N(ctrl, 1);
}

void kb_Prev_Image_By_10(doko_control_t* ctrl) {

    kb_Prev_Image_N(ctrl, 10);
}

void kb_Jump_Image_N(doko_control_t* ctrl, int to) {

    _ZERO_SIZE_WARN(ctrl);

    _NO_IMAGE_WARN(ctrl);

    set_image(ctrl, to);
}

void kb_Jump_Image_Start(doko_control_t* ctrl) {

    kb_Jump_Image_N(ctrl, 0);
}

void kb_Jump_Image_End(doko_control_t* ctrl) {

    kb_Jump_Image_N(ctrl, ctrl->image_files.size - 1);
}

void kb_Print_Debug_Info(doko_control_t* ctrl) {

    doko_image_t* im;

    DARRAY_FOR_EACH(ctrl->image_files, i) {

        im = ctrl->image_files.buffer + i;

        L_I("%zu: %s\n", i, im->path);
    }

    im = ctrl->selected_image;

    _NO_IMAGE_WARN(ctrl);

    L_I("Image[%d]:\n", ctrl->selected_index);
    L_I("   Size     %0.1f %0.1f\n", im->srcRect.width, im->srcRect.height);
    L_I("   Visible  %0.1f %0.1f\n", im->srcRect.width * im->scale,
        im->srcRect.height * im->scale);
    L_I("   Position %0.1f %0.1f\n", im->dstPos.x, im->dstPos.y);
    L_I("   Scale %f\n", im->scale);
}

void kb_Move_Image_Up(doko_control_t* ctrl) {

    _NO_IMAGE_WARN(ctrl);

    doko_moveScrFracImage(ctrl->selected_image, 0, -1 / 5.0);
}

void kb_Move_Image_Down(doko_control_t* ctrl) {

    _NO_IMAGE_WARN(ctrl);

    doko_moveScrFracImage(ctrl->selected_image, 0, 1 / 5.0);
}

void kb_Move_Image_Left(doko_control_t* ctrl) {

    _NO_IMAGE_WARN(ctrl);

    doko_moveScrFracImage(ctrl->selected_image, -1 / 5.0, 0);
}

void kb_Move_Image_Right(doko_control_t* ctrl) {

    _NO_IMAGE_WARN(ctrl);

    doko_moveScrFracImage(ctrl->selected_image, 1 / 5.0, 0);
}

void kb_Move_Image_By_Mouse_Delta(doko_control_t* ctrl) {

    _NO_IMAGE_WARN(ctrl);

    doko_image_t* im = ctrl->selected_image;

    // // ctrl->lastMouseClick is set after taking input now
    // // so we don't need to set it here, and this can work with any input

    // if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
    //     ctrl->lastMouseClick = mp;
    // }

    // if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {

    im->dstPos.x -= ctrl->lastMouseClick.x - GetMouseX();
    im->dstPos.y -= ctrl->lastMouseClick.y - GetMouseY();

    // ctrl->lastMouseClick = mp;
    // }
}

void kb_Center_Image(doko_control_t* ctrl) {

    _NO_IMAGE_WARN(ctrl);

    doko_centerImage(ctrl->selected_image);
}

void kb_Fit_Center_Image(doko_control_t* ctrl) {

    _NO_IMAGE_WARN(ctrl);

    doko_fitCenterImage(ctrl->selected_image);
}

void kb_Flip_Vertical(doko_control_t* ctrl) {

    _NO_IMAGE_WARN(ctrl);

    float hh = GetScreenHeight() / 2.0;

    I_Y(ctrl) = hh + (hh - I_Y(ctrl)) - (I_SCALE(ctrl) * I_HEIGHT(ctrl));

    ImageFlipVertical(&ctrl->selected_image->rayim);

    ctrl->selected_image->rebuildBuff = 1;
}

void kb_Flip_Horizontal(doko_control_t* ctrl) {

    _NO_IMAGE_WARN(ctrl);

    float hw = GetScreenWidth() / 2.0;

    I_X(ctrl) = hw + (hw - I_X(ctrl)) - (I_SCALE(ctrl) * I_WIDTH(ctrl));

    ImageFlipHorizontal(&ctrl->selected_image->rayim);

    ctrl->selected_image->rebuildBuff = 1;
}

void kb_Color_Invert(doko_control_t* ctrl) {

    _NO_IMAGE_WARN(ctrl);

    ctrl->selected_image->rebuildBuff = true;

    int bytes = 4;

    switch (ctrl->selected_image->rayim.format) {

    default:
        ImageColorInvert(&ctrl->selected_image->rayim);
        break;

    // the raylib ImageColorInvert is not optimal for these formats
    case PIXELFORMAT_UNCOMPRESSED_GRAYSCALE:
    case PIXELFORMAT_UNCOMPRESSED_R8G8B8:

        bytes = 3;

        FALLTHROUGH;

    case PIXELFORMAT_UNCOMPRESSED_R8G8B8A8:
    case PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA: 

        unsigned char* pixels = ctrl->selected_image->rayim.data;

        size_t size = bytes * ctrl->selected_image->rayim.width *
                      ctrl->selected_image->rayim.height;

        for (size_t i = 0; i < size; i += (bytes == 4)) {

            pixels[i] = 255 - pixels[i];
            ++i;
            pixels[i] = 255 - pixels[i];
            ++i;
            pixels[i] = 255 - pixels[i];
            ++i;
        }
        break;
    }
}

void kb_Color_Invert_Shader(doko_control_t* ctrl) {

    _NO_IMAGE_WARN(ctrl);

    ctrl->selected_image->applyInvertShader =
        !ctrl->selected_image->applyInvertShader;
}

void kb_Color_Grayscale_Shader(doko_control_t* ctrl) {

    _NO_IMAGE_WARN(ctrl);

    ctrl->selected_image->applyGrayscaleShader =
        !ctrl->selected_image->applyGrayscaleShader;
}

void kb_Increase_FPS(doko_control_t* ctrl) {

    SetTargetFPS(GetFPS() + 1);
}

void kb_Decrease_FPS(doko_control_t* ctrl) {

    SetTargetFPS(GetFPS() - 1);
}

void kb_Cycle_Screen(doko_control_t* ctrl) {

    ctrl->screen++;

    if (ctrl->screen > DOKO_SCREEN__END) {

        ctrl->screen = DOKO_SCREEN__START;
    }
}

void kb_Cycle_Screen_Reverse(doko_control_t* ctrl) {

    ctrl->screen--;

    if (ctrl->screen < DOKO_SCREEN__START) {

        ctrl->screen = DOKO_SCREEN__END;
    }
}

void kb_Toggle_Image_Filelist_Screen(doko_control_t* ctrl) {

    if (ctrl->screen == DOKO_SCREEN_IMAGE) {

        ctrl->screen = DOKO_SCREEN_FILE_LIST;

    } else {

        ctrl->screen = DOKO_SCREEN_IMAGE;
    }
}

void kb_Toggle_Show_Bar(doko_control_t* ctrl) {

    ctrl->config.show_bar = !ctrl->config.show_bar;

    if (ctrl->config.show_bar) {

        info_bar_height = INFO_BAR_HEIGHT;

    } else {

        info_bar_height = 0;
    }
}

void kb_Goto_Image_Screen(doko_control_t* ctrl) {
    ctrl->screen = DOKO_SCREEN_IMAGE;
}

void kb_Goto_File_List_Screen(doko_control_t* ctrl) {
    ctrl->screen = DOKO_SCREEN_FILE_LIST;
}

void kb_Goto_Thumb_Screen(doko_control_t* ctrl) {
    ctrl->screen = DOKO_SCREEN_THUMB_GRID;
}

void kb_Goto_Keybinds_Screen(doko_control_t* ctrl) {
    ctrl->screen = DOKO_SCREEN_KEYBINDS;
}

void kb_Scroll_Keybind_List_Up(doko_control_t* ctrl) {

    if (ctrl->keybindPageScroll == 0) {

        ctrl->keybindPageScroll = KEYBIND_COUNT + MOUSEBIND_COUNT - 1;

        return;
    }

    ctrl->keybindPageScroll--;
}

void kb_Scroll_Keybind_List_Down(doko_control_t* ctrl) {

    ctrl->keybindPageScroll++;

    if (ctrl->keybindPageScroll >= KEYBIND_COUNT + MOUSEBIND_COUNT) {

        ctrl->keybindPageScroll = 0;
    }
}

void kb_Scroll_Thumb_Page_Up(doko_control_t* ctrl) {

    int cols = GetScreenWidth() / THUMBNAIL_SIZE;

    if (cols < ctrl->thumbPageScroll) {

        ctrl->thumbPageScroll -= cols;

    } else {

        ctrl->thumbPageScroll = 0;
    }
}

void kb_Scroll_Thumb_Page_Down(doko_control_t* ctrl) {

    int cols = GetScreenWidth() / THUMBNAIL_SIZE;
    // int rows = 2 * ctrl->image_files.size / cols;

    // L_I("there are %d cols", cols);
    // L_I("there are %d rows", rows);

    ctrl->thumbPageScroll += cols;

    // if (ctrl->thumbPageScroll >= rows) {

    //     ctrl->thumbPageScroll = rows;
    //     L_I("lcoking at rows count");
    // }
}

void kb_Copy_Image_To_Clipboard(doko_control_t* ctrl) {

    _NO_IMAGE_WARN(ctrl);

    doko_copy_image_to_clipboard(ctrl->selected_image);

    ctrl->message = (doko_message_t){
        .message =
            TextFormat("Copied %s to clipboard", ctrl->selected_image->name),
        .free_when_done  = false,
        .show_for_frames = WINDOW_FPS * 3,
    };
}
