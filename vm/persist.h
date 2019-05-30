/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

// Copyright 2018 John Maloney, Bernat Romagosa, and Jens Mönig

// persist.h - Persistent memory
// John Maloney, December 2017

#ifdef __cplusplus
extern "C" {
#endif

// Persistent Memory Records

// Records in persistent memory start with two header words. They have the form:
//	<'R'><record type><id of chunk/variable/comment><extra> (8-bits for each field)
//	word count (32-bits)
//	... word count data words ...
//
// Not all record types use the <extra> header field.

#define PERSISTENT_HEADER_WORDS 2

typedef enum {
	chunkCode = 10,
	chunkAttribute = 11,
	chunkDeleted = 19,

	varValue = 20,
	varName = 21,
	varsClearAll = 29,

	comment = 30,
	commentPosition = 31,
	commentDeleted = 39,

	deleteAll = 218, // 218 in hex is 0xDA, short for "delete all"
} RecordType_t;

// Chunk Attributes

typedef enum {
	sourcePosition = 0,
	snapSourceString = 1,
	gpSourceString = 2,
	ATTRIBUTE_COUNT,
} ChunkAttributeType_t;

// Persistent Memory Operations

int * appendPersistentRecord(int recordType, int id, int extra, int byteCount, uint8 *data);
void clearPersistentMemory();
int * recordAfter(int *lastRecord);
void restoreScripts();

// File-Based Persistent Memory Operations

void initCodeFile(uint8 *flash, int flashByteCount);
void writeCodeFile(uint8 *code, int byteCount);
void writeCodeFileWord(int word);
void clearCodeFile(int cycleCount);

#ifdef __cplusplus
}
#endif
