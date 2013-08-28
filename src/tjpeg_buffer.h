#ifndef _TJPEG_BUFFER_H_

#include <stdint.h>

/**
 * Size of the buffer in bytes
 */
#define TJPEG_BUFFER_SIZE (512)


/**
 * TinyJPEG Buffer structure
 */
typedef struct _tjpeg_buffer_t tjpeg_buffer_t;


  /**
   * Initializes the buffer
   */
  void tjpeg_buffer_init(tjpeg_buffer_t *buffer);


  tjpeg_buffer_t * tjpeg_buffer_new(void);


  /**
   * Returns pointer to first byte of buffer data. Buffer data is always
   * continuous memory chunk.
   * @param buffer  pointer to buffer structure
   * @return pointer to the buffer data
   */
  unsigned char *tjpeg_buffer_get_data(tjpeg_buffer_t *buffer);

  /**
   * Returns length of the buffer in bytes (only fully filled bytes are included)
   * @param buffer  pointer to buffer structure
   * @return length of the buffer
   */
  int tjpeg_buffer_get_length(tjpeg_buffer_t *buffer);


  /**
   * Appends bit string to buffer. Maximum number of bits that can be added
   * depends on sizeof(int) -- it's most significant byte cannot contain data.
   * @param buffer  pointer to buffer structure
   * @param bits    bits to append (uperfluous bits are ignored)
   * @param bits_n  number of bits to add
   */
  void tjpeg_buffer_add(tjpeg_buffer_t *buffer, unsigned int bits, unsigned char bits_n);

  /**
   * Appends to buffer AC huffman code with value. Passed table must contain mapping
   * from symbols to huffman codes. Symbols are defined as length(value) * 16 + run.
   * Codes are in format 0xaaab, where aaa are code bits and b is the length of
   * the code minus 1. If code length of the code is greater than 12, code must
   * be supplemented by appropriate number of 1.
   * @param buffer  pointer to buffer structure
   * @param table   huffman codes map
   * @param run     length of the run of consecutive zeroes
   * @param value   AC value to insert
   */
  void tjpeg_buffer_add_ac(tjpeg_buffer_t *buffer, const uint16_t *table, uint8_t run, int16_t value);

  /**
   * Appends to buffer DC huffman code with value. Passed table must contain mapping
   * from symbols to huffman codes. Symbols are defined as length(value).
   * Codes are in format 0xaaab, where aaa are code bits and b is the length of
   * the code minus 1.
   * @param buffer  pointer to buffer structure
   * @param table   huffman codes map
   * @param value   DC value to insert
   */
  void tjpeg_buffer_add_dc(tjpeg_buffer_t *buffer, const uint16_t *table, int16_t value);

  /**
   * Adds one byte to buffer, after actual write pointer. Remaining bits in current
   * byte are filled with zeroes.
   * @param buffer  pointer to buffer structure
   * @param byte    byte to add to buffer
   */
  void tjpeg_buffer_add_byte(tjpeg_buffer_t *buffer, unsigned char byte);

  void tjpeg_buffer_trunc_bytes(tjpeg_buffer_t *buffer);


#define _TJPEG_BUFFER_H_
#endif
