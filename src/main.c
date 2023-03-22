#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

#include <time.h>
#include <stdlib.h>

#include "stb_image.h"
#include "stb_image_resize.h"
#include "stb_image_write.h"

#include "icer.h"

const char filename[] = "../lena_color.gif";
const char resized_filename[] = "../pic_resized.bmp";
const char resized_6bpp_filename[] = "../pic_resized_6bpp.bmp";
const char wavelet_filename[] = "../wavelet_img.bmp";
const char wavelet_inv_filename[] = "../wavelet_inv_img.bmp";

void reduce_bit_depth(unsigned char *buf, size_t len, uint8_t bits) {
    unsigned char *it = buf;
    for (;it < (buf+len);it++) {
        (*it) >>= bits;
    }
}

void increase_bit_depth(unsigned char *buf, size_t len, uint8_t bits) {
    unsigned char *it = buf;
    for (;it < (buf+len);it++) {
        (*it) <<= bits;
    }
}

bool compare(const unsigned char *buf1, const unsigned char *buf2, size_t len) {
    bool identical = true;
    size_t cnt = 0;
    for (size_t it = 0;it < len;it++) {
        if (buf1[it] != buf2[it]) {
            cnt++;
            identical = false;
        }
    }
    printf("total differences: %zu\n", cnt);

    return identical;
}

int main() {
    setbuf(stdout, 0);
    const size_t out_w = 512;
    const size_t out_h = 512;
    const size_t out_channels = 1;

    int src_w, src_h, n;
    uint8_t *data;

    uint8_t *resized = malloc(out_w*out_h*out_channels);
    uint8_t *compress = malloc(out_w*out_h*out_channels);
    uint8_t *decompress = malloc(out_w*out_h*out_channels);

    int res = 0;
    clock_t begin, end;

    icer_init();

    printf("test compression code\n");

    printf("loading image: \"%s\"\n", filename);
    data = stbi_load(filename, &src_w, &src_h, &n, out_channels);
    if (data == NULL) {
        printf("invalid image\nexiting...\n");
        return 0;
    }

    printf("loaded image\nwidth    : %5d\nheight   : %5d\nchannels : %5d\nout_chn  : %5zu\n", src_w, src_h, n, out_channels);

    printf("resizing image to width: %4zu, height: %4zu\n", out_w, out_h);
    res = stbir_resize_uint8(data, src_w, src_h, 0,
                             resized, out_w, out_h, 0,
                             out_channels);
    if (res == 0) {
        printf("resize failed\nexiting...\n");
        return 0;
    }

    printf("saving resized image to: \"%s\"\n", resized_filename);
    res = stbi_write_bmp(resized_filename, out_w, out_h, out_channels, resized);
    if (res == 0) {
        printf("save failed\nexiting...\n");
        return 0;
    }
    int bit_red = 2;
    reduce_bit_depth(resized, out_w*out_h*out_channels, bit_red);
    increase_bit_depth(resized, out_w*out_h*out_channels, bit_red);

    printf("saving bit-depth reduced image to: \"%s\"\n", resized_6bpp_filename);
    res = stbi_write_bmp(resized_6bpp_filename, out_w, out_h, out_channels, resized);
    if (res == 0) {
        printf("save failed\nexiting...\n");
        return 0;
    }

    memcpy(compress, resized, out_h*out_w*out_channels);

    reduce_bit_depth(compress, out_w*out_h*out_channels, bit_red);

    const int datastream_size = 10000000;
    uint8_t *datastream = malloc(datastream_size);
    output_data_buf_typedef output;
    icer_init_output_struct(&output, datastream, datastream_size);
    icer_compress_image_uint8(compress, out_w, out_h, 3, ICER_FILTER_A, 5, &output);
    size_t decomp_w, decomp_h;
    icer_decompress_image_uint8(decompress, &decomp_w, &decomp_h, out_w*out_h, output.data_start, output.size_used, 3, ICER_FILTER_A, 5);

    printf("saving decompressed image to: \"%s\"\n", wavelet_filename);
    res = stbi_write_bmp(wavelet_filename, out_w, out_h, out_channels, decompress);
    if (res == 0) {
        printf("save failed\nexiting...\n");
        return 0;
    }

    if (compare(compress, resized, out_w*out_h*out_channels)) {
        printf("result is identical\n");
    } else {
        printf("result is different\n");
    }

    free(resized);
    free(compress);
    stbi_image_free(data);

    return 0;
}
