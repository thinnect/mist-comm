/**
 * Fragmentation and assembly functions.
 * Maximum fragment size needs to be known for assembly, all but the last
 * fragment need to be of this size, the last one can be shorter.
 * Maximum number of fragments is 8, maximum data size is 255 bytes.
 *
 * Copyright Thinnect Inc. 2020
 * @license MIT
 */
#ifndef FRAGMENTER_ASSEMBLER_H_
#define FRAGMENTER_ASSEMBLER_H_

#include <string.h>
#include <stdint.h>
#include <stdbool.h>

/**
 * Compute the number of fragments needed to split dataSize over fragments of fragMaxSize.
 * Supports sending empty messages - returns 1 for dataSize 0.
 *
 * @param dataSize - Number of bytes to send.
 * @param fragMaxSize - Free space in one fragment.
 *
 * @return Number of fragments needed to send dataSize bytes of data.
 */
uint8_t data_fragments(uint8_t dataSize, uint8_t fragMaxSize);

/**
 * @return size of fragment
 */
uint8_t data_fragmenter(uint8_t fragment[], uint8_t fragSize, uint8_t offset,
                        uint8_t data[], uint8_t dataSize);

/**
 * @return true, if assembly complete, false if data still missing.
 */
bool data_assembler(uint8_t fragMaxSize, uint8_t object[], uint8_t objectSize,
                    uint8_t fragment[], uint8_t fragSize,
                    uint8_t offset, uint8_t* fragMap);

#endif // FRAGMENTER_ASSEMBLER_H_
