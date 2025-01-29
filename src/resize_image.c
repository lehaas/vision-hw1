#include <math.h>
#include "image.h"
#include <stdio.h>

float nn_interpolate(image im, float x, float y, int c)
{
    return get_pixel(im, round(x), round(y), c);
}

float translate_to_center(int coord, float ratio) {
    // account of 0.5 offset in scaled and original image

    return (coord + 0.5) * ratio - 0.5;
}

image nn_resize(image im, int w, int h)
{

    float x_ratio = im.w / (float) w;
    float y_ratio = im.h / (float) h;

    image im_resize = make_image(w, h, im.c);
    for (int x = 0; x < w; x++) {
        for (int y = 0; y < h; y++){
            for (int c = 0; c < im.c; c++) {
                
                float value = nn_interpolate(im, translate_to_center(x, x_ratio), translate_to_center(y, y_ratio), c);
                set_pixel(im_resize, x, y, c, value);          

            }
        }
    }

    return im_resize;
}

float bilinear_interpolate(image im, float x, float y, int c)
{
    int x1 = floor(x), x2 = ceil(x);
    int y1 = floor(y), y2 = ceil(y);

    float v11 = get_pixel(im, x1, y1, c);
    float v12 = get_pixel(im, x1, y2, c);
    float v21 = get_pixel(im, x2, y1, c);
    float v22 = get_pixel(im, x2, y2, c);

    float v1 = v11 * (x2 - x) + v21 * (x - x1);
    float v2 = v12 * (x2 - x) + v22 * (x - x1);

    return v1 * (y2 - y) + v2 * (y - y1);
}

image bilinear_resize(image im, int w, int h)
{
    float x_ratio = im.w / (float) w;
    float y_ratio = im.h / (float) h;

    image im_resize = make_image(w, h, im.c);
    for (int x = 0; x < w; x++) {
        for (int y = 0; y < h; y++){
            for (int c = 0; c < im.c; c++) {

                float value = bilinear_interpolate(im, translate_to_center(x, x_ratio), translate_to_center(y, y_ratio), c);
                set_pixel(im_resize, x, y, c, value);          
            }
        }
    }

    return im_resize;
}

