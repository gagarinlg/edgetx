/*
 * Copyright (C) OpenTX
 *
 * Source:
 *  https://github.com/opentx/libopenui
 *
 * This file is a part of libopenui library.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 */

#include "libopenui_config.h"
#include "button.h"
#include "font.h"
#include "theme.h"

void Button::onPress()
{
  bool check = (pressHandler && pressHandler());
  windowFlags = check ? windowFlags | BUTTON_CHECKED : windowFlags & ~BUTTON_CHECKED;
  if(lvobj == nullptr)
    return;

  if (! (windowFlags & BUTTON_CHECKED))
    lv_obj_clear_state(lvobj, LV_STATE_CHECKED);
  else
    lv_obj_add_state(lvobj, LV_STATE_CHECKED);
}

#if defined(HARDWARE_KEYS)
void Button::onEvent(event_t event)
{
  TRACE_WINDOWS("%s received event 0x%X", getWindowDebugString("Button").c_str(), event);

  if (enabled && event == EVT_KEY_FIRST(KEY_ENTER)) {
    killEvents(event);
    onKeyPress();
    onPress();
  }
  else {
    FormField::onEvent(event);
  }
}
#endif

#if defined(HARDWARE_TOUCH)
bool Button::onTouchStart(coord_t x, coord_t y)
{
  Window::onTouchStart(x, y);
  return true;
}
#endif

#if defined(HARDWARE_TOUCH)
bool Button::onTouchEnd(coord_t x, coord_t y)
{
  if (enabled) {
    onKeyPress();
    if (!(windowFlags & NO_FOCUS)) {
      setFocus(SET_FOCUS_DEFAULT);
    }
    onPress();
  }
  return true;
}
#endif

void Button::checkEvents()
{
  Window::checkEvents();
  if (checkHandler)
    checkHandler();
}

lv_style_t style_btn;
LvglWidgetFactory textButtonFactory = {lv_btn_create, nullptr};

TextButton::TextButton(FormGroup* parent, const rect_t& rect, std::string text,
          std::function<uint8_t(void)> pressHandler,
          WindowFlags windowFlags,
          LcdFlags textFlags) :
    Button(parent, rect, std::move(pressHandler), windowFlags, textFlags, &textButtonFactory),
    text(std::move(text))
{
  setTextFlags(textFlags | COLOR_THEME_PRIMARY1);
  lv_obj_add_flag(lvobj, LV_OBJ_FLAG_CHECKABLE);
  if (windowFlags & BUTTON_CHECKED)
    lv_obj_add_state(lvobj, LV_STATE_CHECKED);

  lv_style_init(&style_btn);
  lv_style_set_radius(&style_btn, 0);
  lv_obj_set_style_bg_color(lvobj, makeLvColor(COLOR_THEME_SECONDARY2), LV_PART_MAIN);
  lv_obj_set_style_bg_color(lvobj, makeLvColor(COLOR_THEME_ACTIVE), LV_PART_MAIN | LV_STATE_CHECKED);
  lv_obj_set_style_bg_color(lvobj, makeLvColor(COLOR_THEME_ACTIVE), LV_PART_MAIN | LV_STATE_CHECKED | LV_STATE_FOCUSED);
  lv_obj_set_style_bg_color(lvobj, makeLvColor(COLOR_THEME_FOCUS), LV_PART_MAIN | LV_STATE_FOCUSED);
  lv_obj_set_style_bg_opa(lvobj, LV_OPA_100, LV_PART_MAIN);
  lv_obj_set_style_border_width(lvobj, 0, LV_PART_MAIN);
  
  lv_obj_set_style_border_width(lvobj, 1, LV_PART_MAIN | LV_STATE_CHECKED | LV_STATE_FOCUSED);
  lv_obj_set_style_border_color(lvobj, makeLvColor(COLOR_THEME_FOCUS), LV_PART_MAIN | LV_STATE_CHECKED | LV_STATE_FOCUSED);

  lv_obj_set_style_text_color(lvobj, makeLvColor(COLOR_THEME_PRIMARY2), LV_PART_MAIN | LV_STATE_FOCUSED);

  lv_obj_add_style(lvobj, &style_btn,0);

  label = lv_label_create(lvobj);
  lv_label_set_text(label, this->text.c_str());
  lv_obj_center(label);
  lv_obj_set_style_text_color(label, makeLvColor(this->getTextFlags()), LV_PART_MAIN);
  lv_obj_set_style_text_color(label, makeLvColor(COLOR_THEME_PRIMARY2), LV_PART_MAIN | LV_STATE_FOCUSED);
}

void IconButton::paint(BitmapBuffer * dc)
{
  dc->drawBitmap(0, 0, theme->getIcon(icon, checked() ? STATE_PRESSED : STATE_DEFAULT));
}
