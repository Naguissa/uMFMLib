/**
 * Tiny lib to enode and decode FM and MFM data streams
 *
 *
 * @copyright Naguissa
 * @author Naguissa
 * @email naguissa@foroelectro.net
 * @version 0.2.0
 * @created 2018-02-24
 */
#include <Arduino.h>
#include "uMFMLib.h"

/**
 * Constructor
 */
uMFMLib::uMFMLib() {
	reset();
}


/**
 * Resets previous value
 */
void uMFMLib::reset() {
	_last = 1;
	_lastType = UMFMFLIB_LASTTYPE_DATA;
	_syncState = 0;
}



/**
 * Changes codification type
 *
 * @param uint8_t Desired codification type
 */
void uMFMLib::setType(uint8_t t) {
	_type = t;
	reset();
}

/**
 * Encodes one bit in corresponding two bits and returns them is less significant bits of a byte
 *
 * @param bool Bit to encode
 * @return uint8_t The two ending bits are the encoded bits
 */
uint8_t uMFMLib::encode(bool i) {
	uint8_t ret;
	if (_type == UMFMFLIB_TYPE_FM) {
		ret = B00000010 | i;
	} else {
		if (i == 1) { // 1 is always 01
			ret = B00000001;
		} else if (_last == 0) {
			ret = B00000010;
		} else {
			ret = B00000000;
		}
	}
	_last = i;
	return ret;
}

/**
 * Encodes 8 bits in corresponding 16 bits (uint16_t)
 *
 * Note, initial value of return for MFM is 43690, 1010101010101010 in uint16_t form
 *
 * @param uint8_t Bit to encode
 * @return uint16_t Encoded bits
 */
uint16_t uMFMLib::encode(uint8_t i) {
	uint16_t ret;
	if (_type == UMFMFLIB_TYPE_FM) {
		ret = (43690 | (i & B00000001) | ((i & B00000010) << 2) | ((i & B00000100) << 4) | ((i & B00001000) << 6)| ((i & B00010000) << 8) | ((i & B00100000) << 10) | ((i & B01000000) << 12) | ((i & B10000000) << 14));
		_last = i & B00000001;
	} else {
		ret = 0;
		bool bit;
		for (int8_t idx = 14; idx >= 0; idx -= 2) {
			bit = (bool) i & B1000000;
			if (bit) { // 1 is always 01
				ret = ret | (B00000001 << idx);
			} else if (_last == 0) {
				ret = ret | (B00000010 << idx);
			/*
			Skipped because not needed; it's defined as 0)
			} else {
				ret = ret | (B00000000 << idx);
			*/
			}
			i = i << 1;
			_last = bit;
		}
	}
	return ret;
}

/**
 * Encodes 16 bits (uint16_t) in corresponding 32 bits (uint32_t)
 *
 * Note, initial value of return for MFM is 2863311530, 10101010101010101010101010101010 in uint32_t form
 *
 * @param uint16_t Bit to encode
 * @return uint32_t Encoded bits
 */
uint32_t uMFMLib::encode(uint16_t i) {
	uint32_t ret;
	if (_type == UMFMFLIB_TYPE_FM) {
		ret = (2863311530 | (i & B00000001) | ((i & B00000010) << 1) | ((i & B00000100) << 2) | ((i & B00001000) << 3)| ((i & B00010000) << 4) | ((i & B00100000) << 5) | ((i & B01000000) << 6) | ((i & B10000000) << 7) | ((i & B00000001 << 8) << 8) | ((i & B00000001 << 9) << 9) | ((i & B00000001 << 10) << 10) | ((i & B00000001 << 11) << 11) | ((i & B00000001 << 12) << 12) | ((i & B00000001 << 13) << 13) | ((i & B00000001 << 14) << 14) | ((i & B00000001 << 15) << 15));
		_last = i & B00000001;
	} else {
		ret = 0;
		bool bit;
		uint16_t mask = B0000001 << 15;
		for (int8_t idx = 30; idx >= 0; idx -= 2) {
			bit = (bool) i & mask;
			if (bit) { // 1 is always 01
				ret = ret | (B00000001 << idx);
			} else if (_last == 0) {
				ret = ret | (B00000010 << idx);
			/*
			Skipped because not needed; it's defined as 0)
			} else {
				ret = ret | (B00000000 << idx);
			*/
			}
			_last = bit;
			mask = mask >> 1;
		}
	}
	return ret;
}

/**
 * Decodes one bit
 *
 * You need to feed always two bits in {clock, data} order.
 * If bit corresponds to clock, B00000000 is returned.
 * If bit corresponds to data, correct data is returned.
 * No ckecks are done, so if it's invalid will return unexpected results.
 * Process is simple: In both encodings data bit correspond to actual data, so simple ignore clock bits.
 *
 * @param bool Bit to decode
 * @return uint8_t The two ending bits marks decoded data. Bit 6 marks if it's a data bit (0 if it's a clock bit, so should be skipped) and bit 7 is data itself.
 */
uint8_t uMFMLib::decode(bool i) {
	uint8_t ret;
	if (_lastType == UMFMFLIB_LASTTYPE_DATA) {
		_lastType = UMFMFLIB_LASTTYPE_CLOCK;
		ret = B00000000;
	} else {
		_lastType = UMFMFLIB_LASTTYPE_DATA;
		ret = B00000010 | i;
	}
	return ret;
}

/**
 * Decodes 8 bits and returns a byte with 4 less significant bits as data
 *
 * No ckecks are done, so if it's invalid will return unexpected results.
 * Process is simple: In both encodings data bit correspond to actual data, so simple ignore clock bits.
 *
 * @param uint8_t Byte to decode
 * @return uint8_t The four ending bits are the decoded bits
 */
uint8_t uMFMLib::decode(uint8_t i) {
	return B00000000 | (B00000001 & i) | ((B00000100 & i) >> 1) | ((B00010000 & i) >> 2) | ((B01000000 & i) >> 3);
}

/**
 * Decodes 16 bits and returns a byte as data
 *
 * No ckecks are done, so if it's invalid will return unexpected results.
 * Process is simple: In both encodings data bit correspond to actual data, so simple ignore clock bits.
 *
 * @param uint16_t Byte to decode
 * @return uint8_t The four ending bits are the decoded bits
 */
uint8_t uMFMLib::decode(uint16_t i) {
	return B00000000 | (B00000001 & i) | ((B00000100 & i) >> 1) | ((B00010000 & i) >> 2) | ((B01000000 & i) >> 3) | (((B00000001 << 8) & i) >> 4) | (((B00000001 << 10) & i) >> 5) | (((B00000001 << 6) & i) >> 4) | (((B00000001 << 14) & i) >> 7);
}

/**
 * Decodes 32 bits and returns 16 bits as data
 *
 * No ckecks are done, so if it's invalid will return unexpected results.
 * Process is simple: In both encodings data bit correspond to actual data, so simple ignore clock bits.
 *
 * @param uint32_t Byte to decode
 * @return uint16_t The four ending bits are the decoded bits
 */
uint16_t uMFMLib::decode(uint32_t i) {
	return ((uint16_t) 0) | (B00000001 & i) | ((B00000100 & i) >> 1) | ((B00010000 & i) >> 2) | ((B01000000 & i) >> 3) | (((B00000001 << 8) & i) >> 4) | (((B00000001 << 10) & i) >> 5) | (((B00000001 << 6) & i) >> 4) | (((B00000001 << 14) & i) >> 7) | (((B00000001 << 16) & i) >> 8) | (((B00000001 << 18) & i) >> 9) | (((B00000001 << 20) & i) >> 10) | (((B00000001 << 22) & i) >> 11) | (((B00000001 << 24) & i) >> 12) | (((B00000001 << 26) & i) >> 13) | (((B00000001 << 28) & i) >> 14) | (((B00000001 << 30) & i) >> 15);
}



/**
 * Waits for sync flag
 *
 * You feed it with read bits and returns true only 1st tyme that sync flag is detected
 *
 * @param bool Read bit
 * @return bool True if just detected
 */
bool uMFMLib::waitForSync(bool i) {
	bool ret = false;

	// State machine checking coding depending in actual bit + previous ones (these ones are state-dependant, not in memory)
	switch (_syncState) {
		case 15: // ended
			break;

		case 14: // 1
			if (syncMark[_syncState] == i) {
				ret = true;
				_syncState++;
			} else {
				_syncState = 4;
			}
			break;

		case 7: // 1
			if (syncMark[_syncState] == i) {
				_syncState++;
			} else {
				_syncState = 4;
			}
			break;

		case 11: // 1
		case 4: // 1
			if (syncMark[_syncState] == i) {
				_syncState++;
			} else {
				_syncState = 0;
			}
			break;

		case 13: // 0
		case 12: // 0
		case 10: // 0
		case 9: // 0
		case 8: // 0
		case 6: // 0
		case 5: // 0
		case 3: // 0
		case 2: // 0
		case 1: // 0
			if (syncMark[_syncState] == i) {
				_syncState++;
			} else {
				_syncState = 1;
			}
			break;

		case 0: // 1
		default:
			if (syncMark[_syncState] == i) {
				_syncState++;
			}
			break;
	}

	return ret;
}



