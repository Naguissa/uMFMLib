/**
 * Tiny lib to enode and decode FM and MFM data streams
 *
 *
 * @copyright Naguissa
 * @author Naguissa
 * @email naguissa@foroelectro.net
 * @version 0.1.0
 * @created 2018-02-24
 */
#ifndef _UMFMLIB_
	#define _UMFMLIB_
	#include "Arduino.h"


	#define UMFMFLIB_TYPE_FM 0
	#define UMFMFLIB_TYPE_MFM 1


	#define UMFMFLIB_LASTTYPE_DATA true
	#define UMFMFLIB_LASTTYPE_CLOCK false

	class uMFMLib {
		public:
			// Constructors
			uMFMLib();
			void reset();
			void setType(unsigned char);

			unsigned char encode(bool);
			unsigned char decode(bool);

			const bool syncMark[15] = {1,0,0,0,1,0,0,1,0,0,0,1,0,0,1}; // 1st bit is missing

			bool waitForSync(bool);

		private:
			// Adresses
			bool _last;
			bool _lastType;
			unsigned char _type = UMFMFLIB_TYPE_MFM;

			unsigned char _syncState;




	};


#endif


