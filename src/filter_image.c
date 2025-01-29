#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "image.h"
#define TWOPI 6.2831853

image l1_normalize(image im) {
  float sum = 0;
  for (size_t i = 0; i < im.c * im.h * im.w; i++) {
    sum += im.data[i];
  }

  for (size_t i = 0; i < im.c * im.h * im.w; i++) {
    im.data[i] /= sum;
  }
  return im;
}

image make_box_filter(int w) {
  image filter = make_image(w, w, 1);
  for (size_t i = 0; i < filter.c * filter.h * filter.w; i++) {
    filter.data[i] = 1;
  }
  filter = l1_normalize(filter);
  return filter;
}

float convolute(image im, int x, int y, int c, image filter, int c_filter) {
  float sum = 0.0;
  assert(filter.w == filter.h);
  int pivot = filter.w / 2;
  for (int px = 0; px < filter.h; px += 1) {
    for (int py = 0; py < filter.w; py += 1) {
      float im_pixel = get_pixel(im, x + px - pivot, y + py - pivot, c);
      float filter_pixel = get_pixel(filter, px, py, c_filter);
      // printf("im_pixel[%d][%d][%d] = %f, filter_pixel[%d][%d][%d] = %f\n",
      //        x + px - pivot, y + py - pivot, c, im_pixel, px, py, c_filter,
      //        filter_pixel);
      sum += im_pixel * filter_pixel;
      // printf("sum after adding pixel[%d][%d] = %f\n", px, py, sum);
    }
  }
  // printf("Final sum = %f\n", sum);
  // exit(0);
  return sum;
}

float fclamp(float d, float min, float max) {
  d = d < min ? min : d;
  return d > max ? max : d;
}

image convolve_image(image im, image filter, int preserve) {
  /*
  Apply a convolutional filter to the image.

  If filter and im have the same number of channels then it's just a normal
  convolution. We sum over spatial and channel dimensions and produce a 1
  channel image. UNLESS: If preserve is set to 1 we should produce an image with
  the same number of channels as the input. This is useful if, for example, we
  want to run a box filter over an RGB image and get out an RGB image. This
  means each channel in the image will be filtered by the corresponding channel
  in the filter. UNLESS: If the filter only has one channel but im has multiple
  channels we want to apply the filter to each of those channels. Then we either
  sum between channels or not depending on if preserve is set.
  */

  assert((im.c == filter.c) || (filter.c == 1));

  image result = (preserve == 1) ? make_image(im.w, im.h, im.c)
                                 : make_image(im.w, im.h, 1);

  convolute(im, 2, 0, 0, filter, (filter.c == 1) ? 0 : 0);
  for (int x = 0; x < im.w; x++) {
    for (int y = 0; y < im.h; y++) {
      if (preserve == 1) {
        // preserve the input image size
        for (int c = 0; c < im.c; c++) {
          float value = convolute(im, x, y, c, filter, (filter.c == 1) ? 0 : c);
          // value = fclamp(value, 0, 1.0);
          set_pixel(result, x, y, c, value);
        }
      } else {
        // flatten the output to a single channel
        float value = 0;
        for (int c = 0; c < im.c; c++) {
          value += convolute(im, x, y, c, filter, (filter.c == 1) ? 0 : c);
        }
        // value = fclamp(value, 0, 1.0);
        set_pixel(result, x, y, 0, value);
      }
    }
  }
  return result;
}

image make_highpass_filter() {
  image f = make_image(3, 3, 1);
  float filter_data[9] = {0, -1, 0, -1, 4, -1, 0, -1, 0};
  memcpy(f.data, filter_data, 9 * sizeof(float));
  return f;
}

image make_sharpen_filter() {
  image f = make_image(3, 3, 1);
  float filter_data[9] = {0, -1, 0, -1, 5, -1, 0, -1, 0};
  memcpy(f.data, filter_data, 9 * sizeof(float));
  return f;
}

image make_emboss_filter() {
  image f = make_image(3, 3, 1);
  float filter_data[9] = {-2, -1, 0, -1, 1, 1, 0, 1, 2};
  memcpy(f.data, filter_data, 9 * sizeof(float));
  return f;
}

// Question 2.2.1: Which of these filters should we use preserve when we run our
// convolution and which ones should we not? Why? Answer: TODO

// highpass (preserve = 0) it detects edges
// sharpen (preserve = 1) it detects edges
// emboss (both) is possible if we preserve, the color are smoothed and pike at
// color changes

// Question 2.2.2: Do we have to do any post-processing for the above filters?
// Which ones and why? Answer: TODO
// highpass: yes -> detect smoother edges
// sharpen: no, gaussion would smoothen out the sharpening
// emboss: yes for 1 channel, smooth out color changes, 3-channel, not sure

float gaussian(float x, float y, float sigma) {
  return 1 / (2 * M_PI * sigma * sigma) *
         exp((-(x * x + y * y) / (2 * sigma * sigma)));
}

image make_gaussian_filter(float sigma) {
  int w = ceil(sigma * 6) + 1;
  image filter = make_image(w, w, 1);
  for (int x = 0; x < w; x++) {
    for (int y = 0; y < w; y++) {
      set_pixel(filter, x, y, 0, gaussian(x - w / 2, y - w / 2, sigma));
    }
  }
  filter = l1_normalize(filter);
  return filter;
}

image add_image(image a, image b) {
  assert((a.h == b.h) & (a.w == b.w) & (a.c == b.c));

  image im = make_image(a.w, a.h, a.c);

  for (int i = 0; i < a.h * a.w * a.c; i++) {
    im.data[i] = a.data[i] + b.data[i];
  }

  return im;
}

image sub_image(image a, image b) {
  assert((a.h == b.h) & (a.w == b.w) & (a.c == b.c));

  image im = make_image(a.w, a.h, a.c);

  for (int i = 0; i < a.h * a.w * a.c; i++) {
    im.data[i] = a.data[i] - b.data[i];
  }

  return im;
}

image make_gx_filter() {
  image f = make_image(3, 3, 1);
  float filter_data[9] = {-1, 0, 1, -2, 0, 2, -1, 0, 1};
  memcpy(f.data, filter_data, 9 * sizeof(float));
  return f;
}

image make_gy_filter() {
  image f = make_image(3, 3, 1);
  float filter_data[9] = {-1, -2, -1, 0, 0, 0, 1, 2, 1};
  memcpy(f.data, filter_data, 9 * sizeof(float));
  return f;
}

void feature_normalize(image im) {
  float min = 1.0;
  float max = 0.0;
  for (int i = 0; i < im.h * im.w * im.c; i++) {
    min = min > im.data[i] ? im.data[i] : min;
    max = max < im.data[i] ? im.data[i] : max;
  }
  float range = max - min;
  for (int i = 0; i < im.h * im.w * im.c; i++) {
    im.data[i] = (im.data[i] - min) / (range == 0.0 ? 1.0 : range);
  }
}

image *sobel_image(image im) {
  image magnitude = make_image(im.w, im.h, 1);
  image direction = make_image(im.w, im.h, 1);

  image conv_x = convolve_image(im, make_gx_filter(), 0);
  image conv_y = convolve_image(im, make_gy_filter(), 0);
  // printf("conv_x: w = %d, h = %d, c = %d\n", conv_x.w, conv_x.h, conv_x.c);
  // printf("conv_y: w = %d, h = %d, c = %d\n", conv_y.w, conv_y.h, conv_y.c);

  for (int i = 0; i < im.w * im.h; i++) {
    float x = conv_x.data[i];
    float y = conv_y.data[i];
    magnitude.data[i] = sqrt(x * x + y * y);
    direction.data[i] = atan2(y, x);
  }

  image *images = calloc(2, sizeof(image));
  images[0] = magnitude;
  images[1] = direction;
  free_image(conv_x);
  free_image(conv_y);
  return images;
}

image colorize_sobel(image im) {
  image *images = sobel_image(im);
  image mag = images[0];
  image dir = images[1];

  image im_out = make_image(im.w, im.h, 3);

  feature_normalize(mag);
  feature_normalize(dir);

  for (int x = 0; x < im_out.w; x++) {
    for (int y = 0; y < im_out.h; y++) {
      set_pixel(im_out, x, y, 0, get_pixel(dir, x, y, 0));
      set_pixel(im_out, x, y, 1, get_pixel(mag, x, y, 0));
      set_pixel(im_out, x, y, 2, get_pixel(mag, x, y, 0));
    }
  }
  hsv_to_rgb(im_out);
  for (int i = 0; i < im_out.h * im_out.w * im_out.c; i++) {
    im_out.data[i] = fclamp(im_out.data[i], 0.0, 1.0);
  }
  return im_out;
}
