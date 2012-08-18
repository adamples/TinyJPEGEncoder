#ifndef _TJPEG_IMAGE_CHUNK_

#include <stdint.h>
#include <stdbool.h>


typedef struct _tjpeg_image_chunk_t {
  uint8_t   *data;
  int       width;
  int       height;
  int       x_pos;
  int       y_pos;
} tjpeg_image_chunk_t;


  void tjpeg_image_chunk_init(tjpeg_image_chunk_t *chunk, uint8_t *data, int width, int height);

  bool tjpeg_image_chunk_next_block(tjpeg_image_chunk_t *chunk);

  void tjpeg_image_chunk_copy_y1(tjpeg_image_chunk_t *chunk, int16_t *destination);
  void tjpeg_image_chunk_copy_y2(tjpeg_image_chunk_t *chunk, int16_t *destination);
  void tjpeg_image_chunk_copy_cr(tjpeg_image_chunk_t *chunk, int16_t *destination);
  void tjpeg_image_chunk_copy_cb(tjpeg_image_chunk_t *chunk, int16_t *destination);


#define _TJPEG_IMAGE_CHUNK_
#endif
