#ifndef _TINY_JPEG_H_

#include <stdbool.h>
#include <stdint.h>


/**
 * @file Tiny JPEG encoder
 *
 * @example
 * #define IMAGE_WIDTH 640
 * #define IMAGE_HEIGHT 480
 *
 * int main(void) {
 *   char          image[IMAGE_WIDTH * IMAGE_HEIGHT * 2];
 *   jpeg_proc_t   processor;
 *   jpeg_block_t  block;
 *
 *   for (int y = 0; y < IMAGE_HEIGHT; y += 20) {
 *     tjp_feed_data(&processor, IMAGE_WIDTH, IMAGE_HEIGHT, &image);
 *
 *     while (tjp_next_block(&processor, &block)) {
 *       // write_block(...)
 *     }
 *   }
 * }
 */


/** Start of Image */
#define MARKER_SOI  (0xd8)

/** JFIF application segment */
#define MARKER_APP0 (0xe0)

/** Quantization Table */
#define MARKER_DQT  (0xdb)

/** Start of Frame */
#define MARKER_SOF0 (0xc0)

/** Huffman Table */
#define MARKER_DHT  (0xc4)

/** Start of Scan */
#define MARKER_SOS  (0xda)

/** End of Image */
#define MARKER_EOI  (0xd9)


/**
 * Image processor object. This structure contains no public members.
 */
typedef struct _jpeg_proc_t {
  uint16_t  image_width;
  uint16_t  image_height;
  uint16_t  x_pos;
  uint16_t  y_pos;
  uint16_t  data_width;
  uint8_t   *data;
  uint8_t   data_x_pos;
  uint8_t   buffer[32];   /* Word is 8-bit wide */
  uint8_t   write_offset; /* To which word in buffer to write */
  uint8_t   bit_offset;   /* To which bit in word to write */
  uint8_t   read_offset;  /* Which word from buffer to read */
  int16_t   last_y_dc;
  int16_t   last_cr_dc;
  int16_t   last_cb_dc;
} jpeg_proc_t;


  uint16_t tjpeg_get_header_size(void);
  uint8_t  tjpeg_get_header(void);

  void tjpeg_init(jpeg_proc_t *processor, uint16_t width, uint16_t height);

  /**
   * Adds data to be encoded to processor.
   * @param processor   processor object (may be not initialized at all)
   * @param width       width of image data [pixels]
   * @param height      height of image data [pixels]; must be 8 unless on edge
   *                    of image, where remaining lines wil be automatically
   *                    filled
   * @param data        image data in 4:2:2 YCrCb format (size must be 2 * width * height)
   */
  void tjpeg_feed_data(jpeg_proc_t *processor, uint16_t width, uint8_t *data);

  /**
   * Encodes next block.
   * @param processor   processor object (may be not initialized at all)
   * @param block       previously allocated buffer for next 3 blocks of data
   * @return            true if block was encoded and stored in buffer, false if
   *                    no more blocks left to compress
   */
  int tjpeg_write(jpeg_proc_t *processor, uint8_t* buffer, int buffer_size);


#define _TINY_JPEG_H_
#endif
