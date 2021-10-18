/*
 * Copyright (C) EdgeTX
 *
 * Based on code named
 *   opentx - https://github.com/opentx/opentx
 *   th9x - http://code.google.com/p/th9x
 *   er9x - http://code.google.com/p/er9x
 *   gruvin9x - http://code.google.com/p/gruvin9x
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef _SDCARD_COMMON_H_
#define _SDCARD_COMMON_H_

#include "ff.h"

#define DEFAULT_CATEGORY "Models"

#define MODEL_FILENAME_PREFIX    "model"
#if defined(SDCARD_RAW)
#define MODEL_FILENAME_SUFFIX    ".bin"
#elif defined(SDCARD_YAML)
#define MODEL_FILENAME_SUFFIX    ".yml"
#endif
#define DEFAULT_MODEL_FILENAME   MODEL_FILENAME_PREFIX "1" MODEL_FILENAME_SUFFIX
#define MODEL_FILENAME_PATTERN   MODEL_FILENAME_PREFIX MODEL_FILENAME_SUFFIX

//#if !defined(EEPROM)
extern ModelHeader modelHeaders[MAX_MODELS];
//#endif

// opens radio.bin or model file
const char* openFileBin(const char* fullpath, FIL* file, uint16_t* size,
                        uint8_t* version);

const char* writeFileBin(const char* fullpath, const uint8_t* data,
                         uint16_t size);

void getModelPath(char * path, const char * filename);

const char * readModel(const char * filename, uint8_t * buffer, uint32_t size, uint8_t * version);
const char * loadModel(const char * filename, bool alarms=true);
const char * createModel();
const char * writeModel();

const char * loadRadioSettings();
const char * writeGeneralSettings();

const char * loadRadioSettings(const char * path);
const char * loadRadioSettings();

void checkModelIdUnique(uint8_t index, uint8_t module);

#endif // _SDCARD_RAW_H_
