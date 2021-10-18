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

#include "opentx.h"
#include "storage.h"
#include "sdcard_common.h"
#include "modelslist.h"
#include "conversions/conversions.h"
#include "model_init.h"

//#if !defined(EEPROM_SDCARD)
ModelHeader modelHeaders[MAX_MODELS];
//#endif

// defined either in sdcard_raw.cpp or sdcard_yaml.cpp
void storageCreateModelsList();

void getModelPath(char * path, const char * filename)
{
  strcpy(path, STR_MODELS_PATH);
  path[sizeof(MODELS_PATH)-1] = '/';
  strcpy(&path[sizeof(MODELS_PATH)], filename);
}

void storageEraseAll(bool warn)
{
  TRACE("storageEraseAll");

#if defined(COLORLCD)
  // the theme has not been loaded before
  static_cast<OpenTxTheme*>(theme)->load();
#endif

  // Init backlight mode before entering alert screens
  requiredBacklightBright = BACKLIGHT_FORCED_ON;

#if defined(PCBHORUS)
  g_eeGeneral.blOffBright = 20;
#endif

  if (warn) {
    ALERT(STR_STORAGE_WARNING, STR_BAD_RADIO_DATA, AU_BAD_RADIODATA);
  }

  RAISE_ALERT(STR_STORAGE_WARNING, STR_STORAGE_FORMAT, NULL, AU_NONE);

  storageFormat();
  storageDirty(EE_GENERAL);
  storageCheck(true);
}

void storageFormat()
{
  sdCheckAndCreateDirectory(RADIO_PATH);
  sdCheckAndCreateDirectory(MODELS_PATH);
  storageCreateModelsList();
  generalDefault();
}

void storageCheck(bool immediately)
{
  if (storageDirtyMsk & EE_GENERAL) {
    TRACE("eeprom write general");
    storageDirtyMsk &= ~EE_GENERAL;
    const char * error = writeGeneralSettings();
    if (error) {
      TRACE("writeGeneralSettings error=%s", error);
    }
  }

  if (storageDirtyMsk & EE_MODEL) {
    TRACE("eeprom write model");
    storageDirtyMsk &= ~EE_MODEL;
    const char * error = writeModel();
    if (error) {
      TRACE("writeModel error=%s", error);
    }
  }
}

const char * createModel()
{
  preModelLoad();

  char filename[LEN_MODEL_FILENAME+1];
  memset(filename, 0, sizeof(filename));
  strcpy(filename, MODEL_FILENAME_PATTERN);

  int index = findNextFileIndex(filename, LEN_MODEL_FILENAME, MODELS_PATH);
  if (index > 0) {
    setModelDefaults(index);
    memcpy(g_eeGeneral.currModelFilename, filename, sizeof(g_eeGeneral.currModelFilename));
    storageDirty(EE_GENERAL);
    storageDirty(EE_MODEL);
    storageCheck(true);
  }
  postModelLoad(false);

  return g_eeGeneral.currModelFilename;
}

const char * loadModel(const char * filename, bool alarms)
{
  uint8_t version;
  preModelLoad();

  const char * error = readModel(filename, (uint8_t *)&g_model, sizeof(g_model), &version);
  if (error) {
    TRACE("loadModel error=%s", error);

    // just get some clean memory state in "g_model"
    // so the mixer can run safely
    memset(&g_model, 0, sizeof(g_model));
    applyDefaultTemplate();

    storageCheck(true);
    postModelLoad(false);
    return error;
  }

#if defined(STORAGE_CONVERSIONS)
  if (version < EEPROM_VER) {
    convertModelData(version);
  }
#endif

  postModelLoad(alarms);
  return nullptr;
}

void storageReadAll()
{
  TRACE("storageReadAll");

  // Wipe models list in case
  // it's being reloaded after USB connection
  modelslist.clear();

  if (loadRadioSettings() != nullptr) {
    storageEraseAll(true);
  }

  for (uint8_t i = 0; languagePacks[i] != nullptr; i++) {
    if (!strncmp(g_eeGeneral.ttsLanguage, languagePacks[i]->id, 2)) {
      currentLanguagePackIdx = i;
      currentLanguagePack = languagePacks[i];
      break;
    }
  }

  // and reload the list
  modelslist.load();

  // Current model filename is empty...
  // Let's fix it!
  if (strlen(g_eeGeneral.currModelFilename) == 0) {

    // sizeof(currModelFilename) == LEN_MODEL_FILENAME + 1
    // make sure it is terminated (see doc for strncpy())
    strncpy(g_eeGeneral.currModelFilename, DEFAULT_MODEL_FILENAME, LEN_MODEL_FILENAME);
    g_eeGeneral.currModelFilename[LEN_MODEL_FILENAME] = '\0';

    storageDirty(EE_GENERAL);
    storageCheck(true);
  }
  
  if (loadModel(g_eeGeneral.currModelFilename, false) != nullptr) {
    TRACE("No current model or SD card error");
  }
}

#if !defined(COLORLCD)
void checkModelIdUnique(uint8_t index, uint8_t module)
{
  if (isModuleXJTD8(module))
    return;

  char * warn_buf = reusableBuffer.moduleSetup.msg;

  // cannot rely exactly on WARNING_LINE_LEN so using WARNING_LINE_LEN-2
  size_t warn_buf_len = sizeof(reusableBuffer.moduleSetup.msg) - WARNING_LINE_LEN - 2;
  if (!modelslist.isModelIdUnique(module,warn_buf,warn_buf_len)) {
    if (warn_buf[0] != 0) {
      POPUP_WARNING(STR_MODELIDUSED);
      SET_WARNING_INFO(warn_buf, sizeof(reusableBuffer.moduleSetup.msg), 0);
    }
  }
}
#endif

