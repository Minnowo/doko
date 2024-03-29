

#include <math.h>
#include <raylib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include "doko.h"
#include "config.h"
#include "darray.h"


#ifdef __unix__

#include <sys/wait.h>

#define PIPE_READ 0
#define PIPE_WRITE 1


int doko_load_with_ffmpeg_stdout(const char* path, Image* im) {

    L_I("About to read image using FFMPEG");
    L_D("FFMPEG decode format is " FFMPEG_CONVERT_MIDDLE_FMT);

    int pipefd[2];

    if (pipe(pipefd) < 0) {

        L_W("Load with FFMPEG: Could not create a pipe: %s", strerror(errno));
        return false;
    }

    pid_t child = fork();

    if (child < 0) {
        L_W("Load With FFMPEG: Could not create fork: %s", strerror(errno));
        return false;
    }

    if (child == 0) {
        // in child process

        if (dup2(pipefd[PIPE_WRITE], STDOUT_FILENO) < 0) {

            L_C("Load With FFMPEG (in child): Could not open write end of pipe as stdout: %s", strerror(errno));

            exit(EXIT_FAILURE);

            return false;
        }

        close(pipefd[PIPE_READ]);

        int ret =
            execlp("ffmpeg", "ffmpeg", "-v", FFMPEG_VERBOSITY, "-nostdin",
                   "-hide_banner", "-i", path, "-c:v",
                   FFMPEG_CONVERT_MIDDLE_FMT, "-f", "image2pipe", "-", NULL);

        if (ret < 0) {
            L_C("Load With FFMPEG (in child): FFMPEG Could not be run: %s",
                strerror(errno));

            exit(EXIT_FAILURE);
        }

        exit(EXIT_SUCCESS);

        return false;
    }

    if (close(pipefd[PIPE_WRITE]) < 0) {
        L_W("Load With FFMPEG: Could not close write stream: %s", strerror(errno));
    }

    ssize_t bytesRead;

    dbyte_arr_t data;

    DARRAY_INIT(data, 1024*1024*4);

    while ((bytesRead = read(pipefd[0], data.buffer + data.size,
                             data.length - data.size)) > 0) {

        data.size += bytesRead;

        if(data.size == data.length) {
            DARRAY_APPEND(data, 0);
            data.size--;
        }

        L_D("Load With FFMPEG: Read %f megabytes from stdout", BYTES_TO_MB(data.size));
    }
    L_I("Load With FFMPEG: Read %f megabytes from stdout", BYTES_TO_MB(data.size));

    wait(NULL);
    close(pipefd[0]);

    if (data.size > 0) {
        *im = LoadImageFromMemory("." FFMPEG_CONVERT_MIDDLE_FMT, data.buffer,
                                  data.size);
    }

    DARRAY_FREE(data);

    return im->data != NULL;
}



int doko_load_with_magick_stdout(const char* path, Image* im) {

    L_I("About to read image using ImageMagick Convert");
    L_D("ImageMagick Convert decode format is " MAGICK_CONVERT_MIDDLE_FMT);

    int pipefd[2];

    if (pipe(pipefd) < 0) {
        L_W("Load with ImageMagick Convert: Could not create a pipe: %s", strerror(errno));
        return false;
    }

    pid_t child = fork();

    if (child < 0) {
        L_W("Load With ImageMagick Convert: Could not create fork: %s", strerror(errno));
        return false;
    }

    if (child == 0) {
        // in child process

        if (dup2(pipefd[PIPE_WRITE], STDOUT_FILENO) < 0) {
            L_C("Load With ImageMagick Convert (in child): Could not open write end of pipe as stdout: %s", strerror(errno));

            exit(EXIT_FAILURE);

            return false;
        }

        close(pipefd[PIPE_READ]);

        size_t n;
        char* new_path = doko_strdupn(path, 3, &n);

        if(new_path == NULL) {
            L_C("Could not copy filename to append [0]. Assuming no memory: %s", strerror(errno));

            exit(EXIT_FAILURE);

            return false;
        }

        // since convert doesn't have any other method to tell it you only want the first image
        // we have to do it like this
        new_path[n - 1] = ']';
        new_path[n - 2] = '0';
        new_path[n - 3] = '[';

        int ret = execlp("convert", "convert", "-quiet", new_path,
                         MAGICK_CONVERT_MIDDLE_FMT ":-", NULL);

        free(new_path);

        if (ret < 0) {
            L_C("Load With Image Magick Convert (in child): FFMPEG Could not be run: %s", strerror(errno));
            exit(EXIT_FAILURE);
        }

        exit(EXIT_SUCCESS);
        return false;
    }

    if (close(pipefd[PIPE_WRITE]) < 0) {
        L_W("Load With Image Magick Convert: Could not close write stream: %s", strerror(errno));
    }

    ssize_t bytesRead;

    dbyte_arr_t data;

    DARRAY_INIT(data, 1024*1024*4);

    while ((bytesRead = read(pipefd[0], data.buffer + data.size,
                             data.length - data.size)) > 0) {

        data.size += bytesRead;

        if(data.size == data.length) {
            DARRAY_APPEND(data, 0);
            data.size--;
        }

        L_D("Load With Image Magick Convert: Read %f megabytes from stdout", BYTES_TO_MB(data.size));
    }
    L_I("Load With Image Magick Convert: Read %f megabytes from stdout", BYTES_TO_MB(data.size));

    wait(NULL);
    close(pipefd[0]);

    if (data.size > 0) {
        *im = LoadImageFromMemory("." MAGICK_CONVERT_MIDDLE_FMT, data.buffer, data.size);
    }

    DARRAY_FREE(data);

    return im->data != NULL;
}

#else

int doko_load_with_magick_stdout(const char* path, Image* im) {
    return 0;
}

int doko_load_with_ffmpeg_stdout(const char* path, Image* im) {
    return 0;
}

#endif



int doko_load_with_imlib2(const char* path, Image* im) {

#if (USE_IMLIB2 == 1)

    #include <Imlib2.h>

    L_I("About to read image using Imlib2");

    Imlib_Image image;

    image = imlib_load_image_immediately_without_cache(path);

    if (!image) {

        L_E("Error loading with imlib2: %s", imlib_strerror(imlib_get_error()));

        return 0;
    }

    imlib_context_set_image(image);

    im->width = imlib_image_get_width();
    im->height = imlib_image_get_height();
    im->format  = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;
    im->mipmaps = 1;

    size_t pixels = im->width * im->height;

    uint32_t *argb = imlib_image_get_data();

    if(!argb) {

        return 0;
    }

    // we have to copy the buffer otherwise we can't free from imlib
    im->data = malloc(pixels * sizeof(Color));

    if(!im->data) {

        L_E("Could not make fake pixel data: %s", strerror(errno));

        return 0;
    }

    Color* d = (Color*)im->data;

    for(size_t i = 0; i < pixels; i++) {

        d[i] = (Color){
            .a = (*argb) >> 24,
            .r = (*argb) >> 16,
            .g = (*argb) >> 8,
            .b = (*argb) >> 0,
        };
        argb++;
    }

    imlib_free_image();


    L_I("Loaded with imlib2");

    return 1;

#else 

    return 0;

#endif
}



int doko_loadImage(doko_image_t* image) {

    if(image->status == IMAGE_STATUS_LOADED) {
        return 1;
    }

    if (!(USE_IMLIB2 && doko_load_with_imlib2(image->path, &image->rayim)) &&
        !(USE_MAGICK_CONVERT && doko_load_with_magick_stdout(image->path, &image->rayim)) &&
        !(USE_FFMPEG_CONVERT && doko_load_with_ffmpeg_stdout(image->path, &image->rayim))) {

        image->rayim = LoadImage(image->path);
    }

    if (image->rayim.data == NULL) {

        image->status = IMAGE_STATUS_FAILED;

        L_W("Could not load image %s", image->path);

        return 0;
    }

    image->srcRect = (Rectangle){
        0.0,
        0.0,
        image->rayim.width,
        image->rayim.height,
    };
    image->dstPos = (Vector2){0, 0};
    image->status = IMAGE_STATUS_LOADED;
    doko_fitCenterImage(image);

    return 1;
}

void doko_fitCenterImage(doko_image_t* image) {

    int sw = ImageViewWidth;
    int sh = ImageViewHeight;
    int iw = image->srcRect.width;
    int ih = image->srcRect.height;

    image->scale = fmin((double)sw / iw, (double)sh / ih);

    image->dstPos.x = (sw / 2.0) - (iw * image->scale) / 2.0;
    image->dstPos.y = (sh / 2.0) - (ih * image->scale) / 2.0;
}

void doko_centerImage(doko_image_t* image) {

    int sw = ImageViewWidth;
    int sh = ImageViewHeight;
    int iw = image->srcRect.width;
    int ih = image->srcRect.height;

    image->dstPos.x = (sw / 2.0) - (iw * image->scale) / 2.0;
    image->dstPos.y = (sh / 2.0) - (ih * image->scale) / 2.0;
}

void doko_ensureImageNotLost(doko_image_t *image) {

    int sw = ImageViewWidth - IMAGE_INVERSE_MARGIN_X;
    int sh = ImageViewHeight - IMAGE_INVERSE_MARGIN_X;
    float iw = (-(image->srcRect.width * image->scale)) + IMAGE_INVERSE_MARGIN_X;
    float ih = (-(image->srcRect.height * image->scale)) + IMAGE_INVERSE_MARGIN_Y;

    if (image->dstPos.x > sw) {
        image->dstPos.x = sw;
    } else if (image->dstPos.x < iw) {
        image->dstPos.x = iw;
    }

    if (image->dstPos.y > sh) {
        image->dstPos.y = sh;
    } else if (image->dstPos.y < ih) {
        image->dstPos.y = ih;
    }
}


void doko_moveScrFracImage(doko_image_t *im, double xFrac, double yFrac) {

    im->dstPos.x += ImageViewWidth * xFrac;
    im->dstPos.y += ImageViewHeight * yFrac;
    doko_ensureImageNotLost(im);
}

void doko_zoomImageCenter(doko_image_t* im, double newScale) {

    double beforeZoom = im->scale;
    double afterZoom = newScale;

    if (afterZoom < 0) {
        afterZoom = SMALLEST_SCALE_VALUE;
    }

    #define beforeZoomWidth im->srcRect.width * beforeZoom
    #define beforeZoomHeight im->srcRect.height * beforeZoom

    #define afterZoomWidth im->srcRect.width * afterZoom
    #define afterZoomHeight im->srcRect.height * afterZoom

    im->dstPos.x -= (int)((afterZoomWidth) - (beforeZoomWidth)) >> 1;
    im->dstPos.y -= (int)((afterZoomHeight) - (beforeZoomHeight)) >> 1;
    im->scale = afterZoom;

    doko_ensureImageNotLost(im);
}


void doko_zoomImageOnPoint(doko_image_t* im, double newScale, int x, int y) {

    double beforeZoom = im->scale;
    double afterZoom = newScale;

    if(afterZoom < 0) {
        afterZoom = SMALLEST_SCALE_VALUE;
    }

    double scaleRatio =
        (im->srcRect.width * beforeZoom) / (im->srcRect.width * afterZoom);

    double mouseOffsetX = x - im->dstPos.x;
    double mouseOffsetY = y - im->dstPos.y;

    im->dstPos.x -= (int)((mouseOffsetX / scaleRatio) - mouseOffsetX);
    im->dstPos.y -= (int)((mouseOffsetY / scaleRatio) - mouseOffsetY);
    im->scale = afterZoom;

    doko_ensureImageNotLost(im);
}



void doko_zoomImageCenterFromClosest(doko_image_t* im, bool zoomIn) {

    size_t index;
    BINARY_SEARCH_INSERT_INDEX(ZOOM_LEVELS, ZOOM_LEVELS_SIZE, im->scale, index);

    if (zoomIn) {
        while(index != ZOOM_LEVELS_SIZE && ZOOM_LEVELS[index] <= im->scale) {
            index++;
        }
    } else  {
        while(index != 0 && ZOOM_LEVELS[index] >= im->scale) {
            index--;
        }
    }

    doko_zoomImageCenter(im, ZOOM_LEVELS[index]);
}

void doko_zoomImageOnPointFromClosest(doko_image_t* im, bool zoomIn, int x, int y) {

    size_t index;
    BINARY_SEARCH_INSERT_INDEX(ZOOM_LEVELS, ZOOM_LEVELS_SIZE, im->scale, index);

    if (zoomIn) {
        while(index != ZOOM_LEVELS_SIZE && ZOOM_LEVELS[index] <= im->scale) {
            index++;
        }
    } else {
        while(index != 0 && ZOOM_LEVELS[index] >= im->scale) {
            index--;
        }
    }

    doko_zoomImageOnPoint(im, ZOOM_LEVELS[index], x, y);
}

char *doko_strdupn(const char *str, size_t n, size_t* len_) {

    const size_t len = strlen(str);

    char *newstr = malloc(len + n + 1);

    if (!newstr) {
        return NULL;
    }

    memcpy(newstr, str, len);
    memset(newstr + len, 0, n + 1);

    if (len_ != NULL) {
        *len_ = len + n;
    }

    return newstr;
}
char *doko_strdup(const char *str) {

#ifdef _POSIX_C_SOURCE

    return strdup(str);

#else

    return doko_strdupn(str, 0, NULL);

#endif
}


void set_image(doko_control_t* ctrl, size_t index) {

    if(index >= ctrl->image_files.size) {
        L_D("Cannot set the image index to %zu since it is out of bounds", index);
        return;
    }

    ctrl->selected_image = ctrl->image_files.buffer + index;
    ctrl->selected_index = index;
    ctrl->renderFrames = RENDER_FRAMES;
}


void doko_log(log_level_t level, FILE* stream, const char *fmt, ...) {

#if (LOG_LEVEL != __LOG_LEVEL_NOTHING)

    va_list ap;

    switch (level) {

    case LOG_LEVEL_NOTHING:
        break;

    case LOG_LEVEL_DEBUG:
#if (LOG_LEVEL <= __LOG_LEVEL_DEBUG)
        va_start(ap, fmt);
        printf("[DEBUG] ");
        vfprintf(stream,  fmt, ap);
        putc('\n', stream);
        va_end(ap);
#endif
        break;

    case LOG_LEVEL_INFO:
#if (LOG_LEVEL <= __LOG_LEVEL_INFO)
        va_start(ap, fmt);
        printf("[INFO] ");
        vfprintf(stream,  fmt, ap);
        putc('\n', stream);
        va_end(ap);
#endif
        break;

    case LOG_LEVEL_WARN:
#if (LOG_LEVEL <= __LOG_LEVEL_WARN)
        va_start(ap, fmt);
        printf("[WARNING] ");
        vfprintf(stream, fmt, ap);
        putc('\n', stream);
        va_end(ap);
#endif
        break;

    case LOG_LEVEL_ERROR:
#if (LOG_LEVEL <= __LOG_LEVEL_ERROR)
        va_start(ap, fmt);
        printf("[ERROR] ");
        vfprintf(stream, fmt, ap);
        putc('\n', stream);
        va_end(ap);
#endif
        break;

    case LOG_LEVEL_CRITICAL:
#if (LOG_LEVEL <= __LOG_LEVEL_CRITICAL)
        va_start(ap, fmt);
        printf("[CRITICAL] ");
        vfprintf(stream,  fmt, ap);
        putc('\n', stream);
        va_end(ap);
#endif
        break;
    }

#endif
}
