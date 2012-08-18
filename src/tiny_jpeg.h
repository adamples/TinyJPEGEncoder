#ifndef _TINY_JPEG_H_

#include <stdbool.h>
#include <stdint.h>

#include "tjpeg_buffer.h"
#include "tjpeg_image_chunk.h"


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
  uint16_t        image_width;
  uint16_t        image_height;
  uint16_t        x_pos;
  uint16_t        y_pos;
  int16_t         last_y1_dc;
  int16_t         last_y2_dc;
  int16_t         last_cr_dc;
  int16_t         last_cb_dc;
  bool            eoi;
  int             blocks_n;
  tjpeg_image_chunk_t   chunk;
  tjpeg_buffer_t        buffer;
} jpeg_proc_t;

/**
 * JFIF File header descriptor
 * File header should be copied at the beginnig of output file. 4 bytes need to
 * be changed:
 *   data + image_width_offset      -- high byte of image_width
 *   data + image_width_offset + 1  -- low byte of image_width
 *   data + image_height_offset     -- high byte of image_height
 *   data + image_height_offset + 1 -- low byte of image_height
 */
typedef struct _jpeg_file_header_t {
  uint16_t      length;
  const uint8_t *data;
  uint16_t      image_width_offset;
  uint16_t      image_height_offset;
} jpeg_file_header_t;


  const jpeg_file_header_t  *tjpeg_get_header(void);

  void tjpeg_init(jpeg_proc_t *processor, uint16_t width, uint16_t height);

  /**
   * Adds data to be encoded to processor.
   * @param processor   processor object (may be not initialized at all)
   * @param width       width of image data [pixels]
   * @param height      height of image data [pixels]
   * @param data        image data in 4:2:2 YCrCb format (size must be 2 * width * height)
   */
  void tjpeg_feed_data(jpeg_proc_t *processor, uint16_t width, uint16_t height, uint8_t *data);

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
