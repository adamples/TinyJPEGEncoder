#ifndef _TJPEG_BUFFER_H_

#include <stdint.h>

/**
 * Size of the buffer in bytes
 */
#define TJPEG_BUFFER_SIZE (512)


/**
 * TinyJPEG Buffer structure
 */
typedef struct _tjpeg_buffer_t {
  uint8_t   data[TJPEG_BUFFER_SIZE];  /** Buffer data */
  uint8_t   read_offset;              /** Output pointer (bytes) */
  uint8_t   bit_offset;               /** Output pointer (bits) */
  uint8_t   write_offset;             /** Input pointer */
} tjpeg_buffer_t;


  /**
   * Initializes buffer for reading and writing
   */
  void tjpeg_buffer_init(tjpeg_buffer_t *buffer);

  /**
   * Returns length of the buffer in bytes
   * @param buffer  pointer to buffer structure
   * @return length of the buffer
   */
  uint16_t tjpeg_buffer_get_length(tjpeg_buffer_t *buffer);

  /**
   * Returns length of the buffer in bits
   * @param buffer  pointer to buffer structure
   * @return length of the buffer
   */
  uint16_t tjpeg_buffer_get_bitlength(tjpeg_buffer_t *buffer);

  /**
   * Appends bit string to buffer
   * @param buffer  pointer to buffer structure
   * @param bits    bits to append (uperfluous bits are ignored)
   * @param bits_n  number of bits to add (1 -- 16)
   */
  void tjpeg_buffer_add(tjpeg_buffer_t *buffer, uint16_t bits, uint8_t bits_n);

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
   * Copies number of bytes from buffer to destination in memory.
   * @param buffer        pointer to buffer structure
   * @param destination   pointer to allocated memory block
   * @param bytes_n       number of bytes to copy (must be <= buffer length)
   */
  void tjpeg_buffer_copy(tjpeg_buffer_t *buffer, uint8_t *destination, int bytes_n);

  /**
   * Adds one byte to buffer, after actual write_pointer. Remaining bits in current
   * byte are filled with zeroes.
   * @param buffer  pointer to buffer structure
   * @param byte    byte to add to buffer
   */
  void tjpeg_buffer_add_byte(tjpeg_buffer_t *buffer, uint8_t byte);


#define _TJPEG_BUFFER_H_
#endif
