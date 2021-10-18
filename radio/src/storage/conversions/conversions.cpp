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
#include "conversions.h"

#if defined(SDCARD_RAW) || defined(SDCARD_YAML)
#include "storage/modelslist.h"
#include "storage/sdcard_common.h"
#include "storage/sdcard_raw.h"
#include "storage/sdcard_yaml.h"
#endif

#if !defined(EEPROM_RLC)
static void drawProgressScreen(const char* filename, int progress, int total)
{
#if defined(COLORLCD)
  OpenTxTheme* l_theme = static_cast<OpenTxTheme*>(theme);

  lcd->reset();
  l_theme->drawBackground(lcd);
  lcd->drawText(LCD_W/2, LCD_H/2 - 30, STR_CONVERTING, FONT(XL) | CENTERED | COLOR_THEME_WARNING);
  lcd->drawText(LCD_W/2, LCD_H/2, filename, FONT(STD) | CENTERED | COLOR_THEME_SECONDARY1);

  l_theme->drawProgressBar(lcd,
                           LCD_W / 4,
                           LCD_H / 2 + 40,
                           LCD_W / 2,
                           20,
                           progress, total);

  WDG_RESET();
  lcdRefresh();
#else
  // TODO: BW progress screen
#endif
}

void convertRadioData(int version)
{
  TRACE("convertRadioData(%d)", version);

#if defined(COLORLCD)
  // the theme has not been loaded before
  static_cast<OpenTxTheme*>(theme)->load();

  // Init backlight mode before entering alert screens
  requiredBacklightBright = BACKLIGHT_FORCED_ON;
  g_eeGeneral.blOffBright = 20;
#endif

  RAISE_ALERT(STR_STORAGE_WARNING, STR_SDCARD_CONVERSION_REQUIRE, NULL,
              AU_NONE);

  // Load models list before converting
  modelslist.load();

  unsigned converted = 0;
  auto to_convert = modelslist.getModelsCount() + 1;

  drawProgressScreen(RADIO_FILENAME, converted, to_convert);
  TRACE("converting '%s' (%d/%d)", RADIO_FILENAME, converted, to_convert);

#if STORAGE_CONVERSIONS < 220
  if (version == 219) {
    convertRadioData_219_to_220(g_eeGeneral);
    storageDirty(EE_GENERAL);
  }
#endif
  g_eeGeneral.version = EEPROM_VER;
  storageDirty(EE_GENERAL);
  storageCheck(true);
  converted++;

#if defined(SIMU)
  RTOS_WAIT_MS(200);
#endif

  const char* error = nullptr;
  for (auto category_ptr : modelslist.getCategories()) {
    for (auto model_ptr : *category_ptr) {

      uint8_t model_version;
      char* filename = model_ptr->modelFilename;

      TRACE("converting '%s' (%d/%d)", filename, converted, to_convert);
      drawProgressScreen(filename, converted, to_convert);

      error = readModelBin(filename, (uint8_t *)&g_model, sizeof(g_model), &model_version);
      if (!error) {

        convertModelData(model_version);

#if defined(SDCARD_YAML)
        //TODO: convert filename  & output to YAML directly
        const char* ext = strrchr(filename, '.');
        if ((ext != nullptr) && !strncmp(ext, MODELS_EXT, 4)) {
          memcpy((void*)ext, (void*)YAML_EXT, sizeof(YAML_EXT) - 1);
        }
        if (!strncmp(filename, g_eeGeneral.currModelFilename, LEN_MODEL_FILENAME)) {
          strncpy(g_eeGeneral.currModelFilename, filename, LEN_MODEL_FILENAME+1);
        }
        error = writeModelYaml(filename);
#else
        char path[256];
        getModelPath(path, filename);

        error = writeFileBin(path, (uint8_t *)&g_model, sizeof(g_model));
#endif
        //TODO: what should be done with this error?
      }

      converted++;

#if defined(SIMU)
  RTOS_WAIT_MS(200);
#endif
    }
  }

#if defined(SDCARD_YAML)
  modelslist.save();
  storageDirty(EE_GENERAL);
  storageCheck(true);
#endif
  
  // reload models list
  modelslist.clear();
  modelslist.load();
}
#endif

void convertModelData(int version)
{
  TRACE("convertModelData(%d)", version);

#if STORAGE_CONVERSIONS < 220
  if (version == 219) {
    version = 219;
    convertModelData_219_to_220(g_model);
  }
#endif
}

#if defined(EEPROM) || defined(EEPROM_RLC)
#include "storage/eeprom_common.h"

void eeConvertModel(int id, int version)
{
  eeLoadModelData(id);
  convertModelData(version);
  uint8_t currModel = g_eeGeneral.currModel;
  g_eeGeneral.currModel = id;

#if !defined(EEPROM)
  char filename[LEN_MODEL_FILENAME+1];
  memset(filename, 0, sizeof(filename));
  char* s = strAppend(filename, MODEL_FILENAME_PREFIX);
  s = strAppendUnsigned(s, id + 1, 2);
  s = strAppend(s, MODEL_FILENAME_SUFFIX);
  memcpy(g_eeGeneral.currModelFilename, filename, sizeof(g_eeGeneral.currModelFilename));

  if (!modelslist.getCurrentCategory()) {
    ModelsCategory* cat = modelslist.createCategory("Models", false);
    modelslist.setCurrentCategory(cat);
  }
  ModelCell* model = modelslist.addModel(modelslist.getCurrentCategory(),
                                         filename, false);
  model->setModelName(g_model.header.name);
#endif
  
  storageDirty(EE_MODEL);
  storageCheck(true);

  g_eeGeneral.currModel = currModel;
}

bool eeConvert()
{
  const char *msg = NULL;

  switch (g_eeGeneral.version) {
    case 219:
      msg = "EEprom Data v219";
      break;
    case 220:
      msg = "EEprom Data v220";
      break;
    default:
      return false;
  }

  int conversionVersionStart = g_eeGeneral.version;

  // Information to the user and wait for key press
  g_eeGeneral.backlightMode = e_backlight_mode_on;
  g_eeGeneral.backlightBright = 0;
  g_eeGeneral.contrast = 25;

  ALERT(STR_STORAGE_WARNING, msg, AU_BAD_RADIODATA);

  RAISE_ALERT(STR_STORAGE_WARNING, STR_EEPROM_CONVERTING, NULL, AU_NONE);

  // General Settings conversion
  eeLoadGeneralSettingsData();
  int version = conversionVersionStart;

#if STORAGE_CONVERSIONS < 220
  if (version == 219) {
    version = 220;
    convertRadioData_219_to_220(g_eeGeneral);
  }
#endif
  g_eeGeneral.version = EEPROM_VER;
  storageDirty(EE_GENERAL);
  storageCheck(true);

#if !defined(EEPROM)
  modelslist.clear();
#endif
  
#if LCD_W >= 212
  lcdDrawRect(60, 6*FH+4, 132, 3);
#else
  lcdDrawRect(10, 6*FH+4, 102, 3);
#endif

  // Models conversion
  for (uint8_t id=0; id<MAX_MODELS; id++) {
#if LCD_W >= 212
    lcdDrawSolidHorizontalLine(61, 6*FH+5, 10+id*2, FORCE);
#else
    lcdDrawSolidHorizontalLine(11, 6*FH+5, 10+(id*3)/2, FORCE);
#endif
    lcdRefresh();

#if defined(SIMU)
    RTOS_WAIT_MS(100);
#endif

#if defined(SDCARD_RAW) || defined(SDCARD_YAML)
    if (eeModelExistsRlc(id)) {
#else
    if (eeModelExists(id)) {
#endif
      eeConvertModel(id, conversionVersionStart);
    }
  }

#if !defined(EEPROM)
  modelslist.save();
#endif

  return true;
}
#endif
