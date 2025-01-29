#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#include "image.h"

int clamp(int d, int min, int max) {
  d = d < min ? min : d;
  return d > max ? max : d;
}

int compute_idx(image im, int x, int y, int c) {
  int w = im.w;
  int h = im.h;

  return w * h * c + w * y + x;
}

float get_pixel(image im, int x, int y, int c) {
  x = clamp(x, 0, im.w - 1);
  y = clamp(y, 0, im.h - 1);

  return im.data[compute_idx(im, x, y, c)];
}

void set_pixel(image im, int x, int y, int c, float v) {
  if (x < 0 || x >= im.w || y < 0 || y >= im.h) return;

  im.data[compute_idx(im, x, y, c)] = v;
}

image copy_image(image im) {
  image copy = make_image(im.w, im.h, im.c);
  int size = im.w * im.h * im.c * sizeof(float);
  memcpy(copy.data, im.data, size);
  return copy;
}

image rgb_to_grayscale(image im) {
  assert(im.c == 3);
  image gray = make_image(im.w, im.h, 1);

  for (int x = 0; x < im.w; x++) {
    for (int y = 0; y < im.h; y++) {
      float r = get_pixel(im, x, y, 0);
      float g = get_pixel(im, x, y, 1);
      float b = get_pixel(im, x, y, 2);

      set_pixel(gray, x, y, 0, 0.299 * r + 0.587 * g + 0.114 * b);
    }
  }

  return gray;
}

void shift_image(image im, int c, float v) {
  for (int x = 0; x < im.w; x++) {
    for (int y = 0; y < im.h; y++) {
      set_pixel(im, x, y, c, get_pixel(im, x, y, c) + v);
    }
  }
}

void clamp_image(image im) {
  for (int x = 0; x < im.w; x++) {
    for (int y = 0; y < im.h; y++) {
      for (int c = 0; c < 3; c++) {
        float v = get_pixel(im, x, y, c);
        float clamped = v > 1.0 ? 1.0 : (v < 0.0 ? 0.0 : v);

        set_pixel(im, x, y, c, clamped);
      }
    }
  }
}

// These might be handy
float three_way_max(float a, float b, float c) {
  return (a > b) ? ((a > c) ? a : c) : ((b > c) ? b : c);
}

float three_way_min(float a, float b, float c) {
  return (a < b) ? ((a < c) ? a : c) : ((b < c) ? b : c);
}

void rgb_to_hsv(image im) {
  for (int x = 0; x < im.w; x++) {
    for (int y = 0; y < im.h; y++) {
      float r = get_pixel(im, x, y, 0);
      float g = get_pixel(im, x, y, 1);
      float b = get_pixel(im, x, y, 2);

      float max = three_way_max(r, g, b);
      float val = max;

      float min = three_way_min(r, g, b);
      float c = val - min;

      float sat = (val == 0) ? 0 : (c / val);

      float hue_prime = 0;

      if (c != 0) {
        if (val == r) {
          hue_prime = (g - b) / c;
        } else if (val == g) {
          hue_prime = (b - r) / c + 2;
        } else {
          hue_prime = (r - g) / c + 4;
        }
      }
      float hue = hue_prime / 6;
      if (hue < 0) hue += 1;

      set_pixel(im, x, y, 0, hue);
      set_pixel(im, x, y, 1, sat);
      set_pixel(im, x, y, 2, val);
    }
  }
}

void hsv_to_rgb(image im) {
  for (int x = 0; x < im.w; x++) {
    for (int y = 0; y < im.h; y++) {
      float hue = get_pixel(im, x, y, 0);
      float sat = get_pixel(im, x, y, 1);
      float val = get_pixel(im, x, y, 2);

      float r = 0, g = 0, b = 0;

      if (val != 0) {
        float c = sat * val;
        float min = val - c;

        hue = hue * 6;

        float x = c * (1 - fabs(fmod(hue, 2) - 1));
        if (hue < 1) {
          r = c;
          g = x;
          b = 0;
        } else if (hue < 2) {
          r = x;
          g = c;
          b = 0;
        } else if (hue < 3) {
          r = 0;
          g = c;
          b = x;
        } else if (hue < 4) {
          r = 0;
          g = x;
          b = c;
        } else if (hue < 5) {
          r = x;
          g = 0;
          b = c;
        } else {
          r = c;
          g = 0;
          b = x;
        }
        r += min;
        g += min;
        b += min;
      }
      set_pixel(im, x, y, 0, r);
      set_pixel(im, x, y, 1, g);
      set_pixel(im, x, y, 2, b);
    }
  }
}

void scale_image(image im, int c, float v) {
  for (int x = 0; x < im.w; x++) {
    for (int y = 0; y < im.h; y++) {
      set_pixel(im, x, y, c, get_pixel(im, x, y, c) * v);
    }
  }
}
