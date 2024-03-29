#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "image.h"
#define TWOPI 6.2831853

void l1_normalize(image im)
{
    float divisor = 0;
    int x, y, c;
    for (c = 0; c < im.c; ++c) {
        for (y = 0; y < im.h; ++y) {
            for (x = 0; x < im.w; ++x) {
                divisor += get_pixel(im, x, y, c);
            }
        }
    }
    for (c = 0; c < im.c; ++c) {
        for (y = 0; y < im.h; ++y) {
            for (x = 0; x < im.w; ++x) {
                float pixel = get_pixel(im, x, y, c);
                set_pixel(im, x, y, c, pixel / divisor);
            }
        }
    }
}

image make_box_filter(int w)
{
    image box_filter = make_image(w, w, 1);
    int x, y;
    for (y = 0; y < w; ++y) {
        for (x = 0; x < w; ++x) {
            set_pixel(box_filter, x, y, 0, (float) 1 / (w * w));
        }
    }
    return box_filter;
}

float apply_filter(image im, image filter, int x, int y, int im_c, int filter_c)
{
    float val = 0;
    int offset_x = filter.w / 2;
    int offset_y = filter.h / 2;
    // loop over filter
    int i, j;
    for (j = 0; j < filter.h; j++) {
        for (i = 0; i < filter.w; i++) {
            float filter_pixel = get_pixel(filter, i, j, filter_c);
            float im_pixel = get_pixel(im, x - offset_x + i, y - offset_y + j, im_c);
            val += (filter_pixel * im_pixel);
        }
    }
    return val;
}

image convolve_image(image im, image filter, int preserve)
{
    // filter should have one channel or the same number of channels as im
    assert(im.c == filter.c || filter.c == 1);
    // filter should have an odd size
    assert(filter.h % 2 == 1);
    // initialize convolved image depending on preserve
    image convolved;
    if (preserve == 1) {
        convolved = make_image(im.w, im.h, im.c);
    } else {
        convolved = make_image(im.w, im.h, 1);
    }
    // loop over all pixels in the input image
    int x, y, c;
    for (y = 0; y < convolved.h; ++y) {
        for (x = 0; x < convolved.w; ++x) {
            if (preserve == 1) {
                for (c = 0; c < im.c; ++c) {
                    int filter_c = c;
                    if (filter.c == 1) {
                        filter_c = 0;
                    }
                    float convolved_val = apply_filter(im, filter, x, y, c, filter_c);
                    set_pixel(convolved, x, y, c, convolved_val);
                }
            } else {
                float convolved_val = 0;
                for (c = 0; c < im.c; ++c) {
                    int filter_c = c;
                    if (filter.c == 1) {
                        filter_c = 0;
                    }
                    convolved_val += apply_filter(im, filter, x, y, c, filter_c);
                }
                set_pixel(convolved, x, y, 0, convolved_val);
            }
        }
    }
    return convolved;
}

image make_highpass_filter()
{
    image highpass = make_image(3, 3, 1);
    set_pixel(highpass, 1, 0, 0, -1);
    set_pixel(highpass, 0, 1, 0, -1);
    set_pixel(highpass, 1, 1, 0, 4);
    set_pixel(highpass, 2, 1, 0, -1);
    set_pixel(highpass, 1, 2, 0, -1);
    return highpass;
}

image make_sharpen_filter()
{
    image sharpen = make_image(3, 3, 1);
    set_pixel(sharpen, 1, 0, 0, -1);
    set_pixel(sharpen, 0, 1, 0, -1);
    set_pixel(sharpen, 1, 1, 0, 5);
    set_pixel(sharpen, 2, 1, 0, -1);
    set_pixel(sharpen, 1, 2, 0, -1);
    return sharpen;
}

image make_emboss_filter()
{
    image emboss = make_image(3, 3, 1);
    set_pixel(emboss, 0, 0, 0, -2);
    set_pixel(emboss, 1, 0, 0, -1);
    set_pixel(emboss, 0, 1, 0, -1);
    set_pixel(emboss, 1, 1, 0, 1);
    set_pixel(emboss, 2, 1, 0, 1);
    set_pixel(emboss, 1, 2, 0, 1);
    set_pixel(emboss, 2, 2, 0, 2);
    return emboss;
}

image make_gaussian_filter(float sigma)
{
    double constant = 1 / (TWOPI * sigma * sigma);
    int kernel_size = (int) ceilf(6 * sigma);
    if (kernel_size % 2 == 0) {
        ++kernel_size;
    }
    image gaussian = make_image(kernel_size, kernel_size, 1);
    int offset = kernel_size / 2;
    int x, y;
    for (y = 0; y < kernel_size; ++y) {
        for (x = 0; x < kernel_size; ++x) {
            double coords_val = - (pow(x - offset, 2) + pow(y - offset, 2));
            double val = constant * exp(coords_val / (double) (2 * sigma * sigma));
            set_pixel(gaussian, x, y, 0, (float) val);
        }
    }
    l1_normalize(gaussian);
    return gaussian;
}

image add_image(image a, image b)
{
    assert(a.c == b.c && a.h == b.h && a.w == b.w);
    image new = make_image(a.w, a.h, a.c);
    int x, y, c;
    for (c = 0; c < new.c; ++c) {
        for (y = 0; y < new.h; ++y) {
            for (x = 0; x < new.w; ++x) {
                float val = get_pixel(a, x, y, c) + get_pixel(b, x, y, c);
                set_pixel(new, x, y, c, val);
            }
        }
    }
    return new;
}

image sub_image(image a, image b)
{
    assert(a.c == b.c && a.h == b.h && a.w == b.w);
    image new = make_image(a.w, a.h, a.c);
    int x, y, c;
    for (c = 0; c < new.c; ++c) {
        for (y = 0; y < new.h; ++y) {
            for (x = 0; x < new.w; ++x) {
                float val = get_pixel(a, x, y, c) - get_pixel(b, x, y, c);
                set_pixel(new, x, y, c, val);
            }
        }
    }
    return new;
}

image make_gx_filter()
{
    image gx_filter = make_image(3, 3, 1);
    set_pixel(gx_filter, 0, 0, 0, -1);
    set_pixel(gx_filter, 0, 1, 0, -2);
    set_pixel(gx_filter, 0, 2, 0, -1);
    set_pixel(gx_filter, 2, 0, 0, 1);
    set_pixel(gx_filter, 2, 1, 0, 2);
    set_pixel(gx_filter, 2, 2, 0, 1);
    return gx_filter;
}

image make_gy_filter()
{
    image gy_filter = make_image(3, 3, 1);
    set_pixel(gy_filter, 0, 0, 0, -1);
    set_pixel(gy_filter, 1, 0, 0, -2);
    set_pixel(gy_filter, 2, 0, 0, -1);
    set_pixel(gy_filter, 0, 2, 0, 1);
    set_pixel(gy_filter, 1, 2, 0, 2);
    set_pixel(gy_filter, 2, 2, 0, 1);
    return gy_filter;
}

void feature_normalize(image im)
{
    float min = MAXFLOAT, max = -MAXFLOAT;
    int x, y, c;
    for (c = 0; c < im.c; ++c) {
        for (y = 0; y < im.h; ++y) {
            for (x = 0; x < im.w; ++x) {
                float pixel = get_pixel(im, x, y, c);
                if (pixel < min) {
                    min = pixel;
                }
                if (pixel > max) {
                    max = pixel;
                }
            }
        }
    }
    float range = max - min;
    for (c = 0; c < im.c; ++c) {
        for (y = 0; y < im.h; ++y) {
            for (x = 0; x < im.w; ++x) {
                if (range == 0) {
                    set_pixel(im, x, y, c, 0);
                } else {
                    float pixel = get_pixel(im, x, y, c);
                    pixel = (pixel - min) / range;
                    set_pixel(im, x, y, c, pixel);
                }
            }
        }
    }
}

image *sobel_image(image im)
{
    // allocate the memory to hold the two images (gradient magnitude and orientation)
    image *res = calloc(2, sizeof(image));
    // initialize the gradient filters in the x and y directions
    image gx_filter = make_gx_filter();
    image gy_filter = make_gy_filter();
    // convolve to get the gradients
    image gx = convolve_image(im, gx_filter, 0);
    image gy = convolve_image(im, gy_filter, 0);
    // initialize gradient magnitude and theta images
    image mag = make_image(im.w, im.h, 1);
    image theta = make_image(im.w, im.h, 1);
    // calculate gradient magnitude and theta
    int x, y;
    for (y = 0; y < im.h; ++y) {
        for (x = 0; x < im.w; ++x) {
            float pixel_gx = get_pixel(gx, x, y, 0);
            float pixel_gy = get_pixel(gy, x, y, 0);
            float val_mag = sqrtf(powf(pixel_gx, 2) + powf(pixel_gy, 2));
            float val_theta = atan2f(pixel_gy, pixel_gx);
            set_pixel(mag, x, y, 0, val_mag);
            set_pixel(theta, x, y, 0, val_theta);
        }
    }
    res[0] = mag;
    res[1] = theta;
    return res;
}

image colorize_sobel(image im)
{
    // get the sobel results and normalize
    image *res = sobel_image(im);
    image mag = res[0];
    image theta = res[1];
    feature_normalize(mag);
    feature_normalize(theta);
    // initialize colorized sobel image
    image sobel_c = make_image(im.w, im.h, 3);
    // loop over pixels to assign colors to all three channels
    int x, y;
    for (y = 0; y < im.h; y++) {
        for (x = 0; x < im.w; x++) {
            float h = get_pixel(theta, x, y, 0);
            float s, v;
            s = v = get_pixel(mag, x, y, 0);
            set_pixel(sobel_c, x, y, 0, h);
            set_pixel(sobel_c, x, y, 1, s);
            set_pixel(sobel_c, x, y, 2, v);
        }
    }
    hsv_to_rgb(sobel_c);
    return sobel_c;
}
