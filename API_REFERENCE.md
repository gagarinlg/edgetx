# EdgeTX API Reference

API documentation for major EdgeTX subsystems and interfaces.

## Table of Contents
- [Mixer API](#mixer-api)
- [Audio API](#audio-api)
- [Storage API](#storage-api)
- [Telemetry API](#telemetry-api)
- [GUI API](#gui-api)
- [Lua API](#lua-api)
- [HAL API](#hal-api)

## Mixer API

The mixer is the core of EdgeTX, processing inputs and generating outputs.

### Core Functions

#### `doMixerCalculations()`

Main mixer calculation function called periodically (typically 50Hz+).

```cpp
void doMixerCalculations();
```

**Description:** Processes all mixer lines, applies curves, limits, and generates channel outputs.

**Called from:** Mixer scheduler task

**Side effects:**
- Updates `channelOutputs[]` array
- Processes logical switches
- Executes custom functions
- Updates telemetry sensors

---

#### `evalMixes(uint8_t tick)`

Evaluates mixer lines for current tick.

```cpp
void evalMixes(uint8_t tick);
```

**Parameters:**
- `tick`: Scheduler tick counter

**Returns:** None

**Description:** Iterates through all mixer lines, applies input sources, weights, curves, and differential.

---

#### `applyMixerWeight(int32_t value, int16_t weight)`

Applies mixer weight to a value.

```cpp
int32_t applyMixerWeight(int32_t value, int16_t weight);
```

**Parameters:**
- `value`: Input value (-1024 to 1024)
- `weight`: Weight percentage (-100 to 100)

**Returns:** Weighted value

**Example:**
```cpp
int result = applyMixerWeight(512, 50); // 50% of input
```

---

#### `applyCurve(int32_t value, int8_t curveParam)`

Applies a curve to input value.

```cpp
int32_t applyCurve(int32_t value, int8_t curveParam);
```

**Parameters:**
- `value`: Input value (-1024 to 1024)
- `curveParam`: Curve index or expo value

**Returns:** Transformed value

**Curve Types:**
- Predefined curves (0-15)
- Custom curves (16+)
- Expo (negative values)

---

### Data Structures

#### `MixData`

Defines a single mixer line.

```cpp
struct MixData {
  uint8_t srcRaw;           // Input source (stick, switch, channel)
  int16_t weight;           // Weight (-100 to 100)
  int8_t  curveParam;       // Curve/expo
  uint8_t carryTrim:1;      // Carry trim
  uint8_t noExpo:1;         // Disable expo
  uint8_t mltpx:2;          // Multiplex mode (ADD/MULTIPLY/REPLACE)
  int8_t  offset;           // Offset value
  int16_t swtch;            // Activation switch
  uint8_t flightModes;      // Active flight modes
  int8_t  differential;     // Differential (-100 to 100)
  int8_t  delayUp;          // Slow up delay
  int8_t  delayDown;        // Slow down delay
  int8_t  speedUp;          // Slow up speed
  int8_t  speedDown;        // Slow down speed
  char    name[LEN_EXPOMIX_NAME+1]; // Mixer name
};
```

---

## Audio API

Audio system for voice prompts, tones, and music.

### Core Functions

#### `audioQueue.playFile(const char* filename, uint8_t flags, uint8_t id)`

Play an audio file.

```cpp
void audioQueue.playFile(const char* filename, uint8_t flags = 0, uint8_t id = 0);
```

**Parameters:**
- `filename`: Path to audio file (WAV format)
- `flags`: Playback flags (PLAY_NOW, PLAY_BACKGROUND)
- `id`: Unique ID for priority handling

**Example:**
```cpp
audioQueue.playFile("hello.wav");
audioQueue.playFile("lowbat.wav", PLAY_NOW); // Interrupt current
```

---

#### `audioQueue.playTone(uint16_t freq, uint16_t duration)`

Play a tone.

```cpp
void audioQueue.playTone(uint16_t freq, uint16_t duration);
```

**Parameters:**
- `freq`: Frequency in Hz
- `duration`: Duration in milliseconds

**Example:**
```cpp
audioQueue.playTone(1000, 200); // 1kHz for 200ms
```

---

#### `audioQueue.pause()`

Pause audio playback.

```cpp
void audioQueue.pause();
```

---

#### `audioQueue.resume()`

Resume audio playback.

```cpp
void audioQueue.resume();
```

---

#### `audioQueue.empty()`

Check if audio queue is empty.

```cpp
bool audioQueue.empty();
```

**Returns:** `true` if no audio is queued or playing

---

### Voice Prompts

#### `AUDIO_*` Constants

Predefined voice prompts:

```cpp
#define AUDIO_HELLO              0
#define AUDIO_TIMER_ELAPSED      1
#define AUDIO_LOW_BATTERY        2
#define AUDIO_INACTIVITY_ALARM   3
// ... many more
```

**Usage:**
```cpp
AUDIO_TIMER_ELAPSED();
AUDIO_LOW_BATTERY();
```

---

## Storage API

File system and model storage interface.

### File Operations

#### `sdMount()`

Mount SD card file system.

```cpp
bool sdMount();
```

**Returns:** `true` if successful

---

#### `sdGetFreeSectors()`

Get free space on SD card.

```cpp
uint32_t sdGetFreeSectors();
```

**Returns:** Number of free 512-byte sectors

---

#### `FIL* openFile(const char* path, const char* mode)`

Open a file.

```cpp
FIL* openFile(const char* path, const char* mode);
```

**Parameters:**
- `path`: File path (e.g., "/MODELS/model01.bin")
- `mode`: "r" (read), "w" (write), "a" (append)

**Returns:** File handle or `nullptr` on error

**Example:**
```cpp
FIL* file = openFile("/LOGS/log.txt", "a");
if (file) {
  f_printf(file, "Log entry\n");
  closeFile(file);
}
```

---

#### `closeFile(FIL* file)`

Close an open file.

```cpp
void closeFile(FIL* file);
```

---

### Model Storage

#### `loadModel(uint8_t index)`

Load a model from storage.

```cpp
void loadModel(uint8_t index);
```

**Parameters:**
- `index`: Model slot (0-based)

**Side effects:**
- Loads model into `g_model`
- Initializes curves and mixers
- Resets timers

---

#### `saveModel(uint8_t index)`

Save current model to storage.

```cpp
void saveModel(uint8_t index);
```

**Parameters:**
- `index`: Model slot (0-based)

---

#### `deleteModel(uint8_t index)`

Delete a model from storage.

```cpp
void deleteModel(uint8_t index);
```

---

### Radio Settings

#### `loadRadioSettings()`

Load radio general settings.

```cpp
void loadRadioSettings();
```

**Side effects:** Loads settings into `g_eeGeneral`

---

#### `saveRadioSettings()`

Save radio general settings.

```cpp
void saveRadioSettings();
```

---

## Telemetry API

Telemetry data collection and display.

### Core Functions

#### `setTelemetryValue(uint8_t protocol, uint16_t id, uint8_t instance, int32_t value, uint8_t unit, uint8_t prec)`

Set a telemetry value.

```cpp
void setTelemetryValue(uint8_t protocol, uint16_t id, uint8_t instance, 
                       int32_t value, uint8_t unit, uint8_t prec);
```

**Parameters:**
- `protocol`: Protocol ID (PROTOCOL_TELEMETRY_*)
- `id`: Sensor ID
- `instance`: Sensor instance
- `value`: Value to set
- `unit`: Unit type (UNIT_VOLTS, UNIT_AMPS, etc.)
- `prec`: Decimal precision

**Example:**
```cpp
setTelemetryValue(PROTOCOL_TELEMETRY_FRSKY_SPORT, 
                  0x0210,  // VFAS ID
                  0,       // Instance
                  1234,    // 12.34V
                  UNIT_VOLTS, 
                  2);      // 2 decimal places
```

---

#### `getTelemetryValue(uint8_t index)`

Get a telemetry sensor value.

```cpp
TelemetryItem* getTelemetryValue(uint8_t index);
```

**Parameters:**
- `index`: Telemetry sensor index

**Returns:** Pointer to telemetry item or `nullptr`

---

#### `resetTelemetry()`

Reset telemetry data.

```cpp
void resetTelemetry();
```

---

### Data Structures

#### `TelemetryItem`

Telemetry sensor data.

```cpp
struct TelemetryItem {
  uint16_t id;           // Sensor ID
  uint8_t  instance;     // Instance number
  uint8_t  moduleIdx;    // Module index
  char     label[TELEM_LABEL_LEN]; // Display name
  uint8_t  unit;         // Unit type
  uint8_t  prec;         // Decimal precision
  int32_t  value;        // Current value
  int32_t  valueMin;     // Minimum recorded
  int32_t  valueMax;     // Maximum recorded
  bool     fresh;        // Recently updated
  uint32_t lastReceived; // Timestamp of last update
};
```

---

### Units

Telemetry unit types:

```cpp
enum TelemetryUnit {
  UNIT_RAW,
  UNIT_VOLTS,
  UNIT_AMPS,
  UNIT_MILLIAMPS,
  UNIT_KTS,
  UNIT_METERS_PER_SECOND,
  UNIT_FEET_PER_SECOND,
  UNIT_KMH,
  UNIT_MPH,
  UNIT_METERS,
  UNIT_FEET,
  UNIT_CELSIUS,
  UNIT_FAHRENHEIT,
  UNIT_PERCENT,
  UNIT_MAH,
  UNIT_WATTS,
  UNIT_MILLIWATTS,
  UNIT_DB,
  UNIT_RPMS,
  UNIT_G,
  UNIT_DEGREE,
  UNIT_RADIANS,
  UNIT_MILLILITERS,
  UNIT_FLOZ,
  UNIT_HOURS,
  UNIT_MINUTES,
  UNIT_SECONDS,
  // ... more
};
```

---

## GUI API

User interface widgets and screens (Color LCD).

### Window Classes

#### `Window`

Base window class.

```cpp
class Window {
public:
  Window(Window* parent, const rect_t& rect);
  virtual ~Window();
  
  void setRect(const rect_t& rect);
  void clear();
  void invalidate();
  
  virtual void paint(BitmapBuffer* dc);
  virtual bool onTouchStart(coord_t x, coord_t y);
  virtual bool onTouchEnd(coord_t x, coord_t y);
};
```

---

#### `FormWindow`

Container for form elements.

```cpp
class FormWindow : public Window {
public:
  FormWindow(Window* parent, const rect_t& rect);
  
  void setScrollPositionY(coord_t value);
  coord_t getScrollPositionY();
};
```

---

### Widget Classes

#### `TextButton`

Clickable button with text.

```cpp
class TextButton : public Button {
public:
  TextButton(Window* parent, const rect_t& rect, 
             std::string text, 
             std::function<uint8_t(void)> pressHandler = nullptr);
};
```

**Example:**
```cpp
new TextButton(window, {10, 10, 100, 30}, "Click Me", [=]() {
  // Button clicked
  TRACE("Button pressed");
  return 0;
});
```

---

#### `TextEdit`

Text input field.

```cpp
class TextEdit : public FormField {
public:
  TextEdit(Window* parent, const rect_t& rect, 
           char* value, uint8_t length);
};
```

**Example:**
```cpp
char buffer[16] = "Default";
new TextEdit(window, {10, 10, 200, 30}, buffer, sizeof(buffer));
```

---

#### `NumberEdit`

Numeric input field.

```cpp
class NumberEdit : public FormField {
public:
  NumberEdit(Window* parent, const rect_t& rect,
             int vmin, int vmax,
             std::function<int()> getValue,
             std::function<void(int)> setValue);
};
```

**Example:**
```cpp
new NumberEdit(window, {10, 10, 100, 30}, 
               0, 100, // min, max
               GET_DEFAULT(g_model.mixData[0].weight),
               SET_DEFAULT(g_model.mixData[0].weight));
```

---

#### `Choice`

Dropdown selection.

```cpp
class Choice : public FormField {
public:
  Choice(Window* parent, const rect_t& rect,
         const char* const values[], int vmin, int vmax,
         std::function<int()> getValue,
         std::function<void(int)> setValue);
};
```

---

#### `StaticText`

Non-editable text label.

```cpp
class StaticText : public Window {
public:
  StaticText(Window* parent, const rect_t& rect, 
             std::string text, LcdFlags flags = 0);
};
```

---

### Layout Helpers

#### `FormGridLayout`

Grid-based layout helper.

```cpp
class FormGridLayout {
public:
  FormGridLayout();
  
  void setLabelWidth(coord_t width);
  void nextLine();
  
  rect_t getLabelSlot();
  rect_t getFieldSlot();
  rect_t getLineSlot();
};
```

**Example:**
```cpp
FormGridLayout grid;
grid.setLabelWidth(100);

// Add label and field
new StaticText(window, grid.getLabelSlot(), "Name:");
new TextEdit(window, grid.getFieldSlot(), name, 10);

grid.nextLine();

new StaticText(window, grid.getLabelSlot(), "Value:");
new NumberEdit(window, grid.getFieldSlot(), 0, 100, 
               GET_DEFAULT(value), SET_DEFAULT(value));
```

---

## Lua API

Lua scripting interface. For complete documentation, see https://luadoc.edgetx.org/

### Model Scripts

Model scripts run continuously while the model is active.

```lua
-- /SCRIPTS/MODELS/script.lua

local function run(event)
  -- Called repeatedly
  
  -- Get input value
  local value = getValue("input1")
  
  -- Set output
  setValue("ch1", value * 0.5)
  
  return 0
end

return { run=run }
```

---

### Telemetry Scripts

Display custom telemetry screens.

```lua
-- /SCRIPTS/TELEMETRY/telem.lua

local function init()
  -- Initialize
end

local function run(event)
  -- Draw telemetry screen
  lcd.clear()
  lcd.drawText(10, 10, "Voltage:", 0)
  lcd.drawNumber(100, 10, getValue("VFAS") * 10, PREC2)
  
  return 0
end

return { init=init, run=run }
```

---

### Widget Scripts

For color LCD radios.

```lua
-- /SCRIPTS/WIDGETS/widget.lua

local function create(zone, options)
  return { zone=zone, options=options }
end

local function update(widget, options)
  widget.options = options
end

local function refresh(widget)
  -- Draw widget in zone
  lcd.drawText(widget.zone.x, widget.zone.y, "Widget")
end

return { name="MyWidget", create=create, update=update, refresh=refresh }
```

---

### Key Functions

#### `getValue(name)`

Get input/channel/telemetry value.

```lua
local stickValue = getValue("rud")  -- Rudder stick
local ch1 = getValue("ch1")         -- Channel 1
local vbat = getValue("VFAS")       -- Telemetry
```

---

#### `setValue(name, value)`

Set channel output value.

```lua
setValue("ch10", 1024)  -- Set CH10 to max
```

---

#### `playFile(filename)`

Play audio file.

```lua
playFile("/SOUNDS/en/hello.wav")
```

---

#### `playTone(freq, duration, pause, flags)`

Play a tone.

```lua
playTone(1000, 200, 0, PLAY_NOW)
```

---

## HAL API

Hardware Abstraction Layer for board-specific functions.

### LED Control

```cpp
void LED_RED_ON();
void LED_RED_OFF();
void LED_GREEN_ON();
void LED_GREEN_OFF();
void LED_BLUE_ON();
void LED_BLUE_OFF();
```

---

### Power Management

```cpp
void boardPowerOn();
void boardPowerOff();
bool isPowerOn();
uint16_t getBatteryVoltage();  // Returns voltage in 0.01V
```

---

### Keys/Switches

```cpp
uint32_t readKeys();
uint32_t readTrims();
uint8_t keyState(uint8_t key);
```

---

### LCD

```cpp
void lcdInit();
void lcdRefresh();
void lcdClear(uint16_t color);
void lcdDrawPixel(uint16_t x, uint16_t y, uint16_t color);
void lcdDrawRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
```

---

### ADC

```cpp
void adcInit();
uint16_t getAnalogValue(uint8_t index);
```

---

### Timers

```cpp
uint32_t getTmr1ms();   // 1ms timer
uint32_t getTmr10ms();  // 10ms timer
```

---

## Data Access Macros

### GET/SET Macros

Convenience macros for getter/setter functions:

```cpp
// GET_DEFAULT: Create getter
GET_DEFAULT(g_model.mixData[0].weight)
// Expands to: [=]() { return g_model.mixData[0].weight; }

// SET_DEFAULT: Create setter
SET_DEFAULT(g_model.mixData[0].weight)
// Expands to: [=](int value) { g_model.mixData[0].weight = value; }

// GET_SET_DEFAULT: Both getter and setter
GET_SET_DEFAULT(g_model.mixData[0].weight)
```

**Usage in GUI:**
```cpp
new NumberEdit(window, rect, 0, 100,
               GET_DEFAULT(g_model.mixData[0].weight),
               SET_DEFAULT(g_model.mixData[0].weight));
```

---

## Global Variables

### Current Model

```cpp
extern ModelData g_model;  // Currently active model
```

**Access:**
```cpp
g_model.modelName
g_model.mixData[0]
g_model.expoData[0]
g_model.logicalSw[0]
```

---

### Radio Settings

```cpp
extern GeneralSettings g_eeGeneral;  // Radio general settings
```

**Access:**
```cpp
g_eeGeneral.contrast
g_eeGeneral.vBatWarn
g_eeGeneral.backlightMode
```

---

### Channel Outputs

```cpp
extern int16_t channelOutputs[MAX_OUTPUT_CHANNELS];
```

**Access:**
```cpp
int16_t ch1Value = channelOutputs[0];  // -1024 to 1024
```

---

## Additional Resources

- [Lua Documentation](https://luadoc.edgetx.org/) - Complete Lua API
- [ARCHITECTURE.md](ARCHITECTURE.md) - System architecture
- [DEVELOPER_GUIDE.md](DEVELOPER_GUIDE.md) - Development guide
- [Source Code](radio/src/) - Always refer to source for latest API

---

**Note:** This API reference covers the most commonly used functions. For complete API details, refer to the source code header files in `radio/src/`.
