# TinyBit Virtual Console

TinyBit is a virtual console for creating and playing simple 2D games using Lua scripting. Games are stored as PNG cartridge files that embed both the spritesheet assets and Lua script data.

## Features

- **128x128 pixel display** with 4-bit RGBA color depth
- **Lua scripting** for game logic with built-in graphics, input, and audio APIs
- **PNG-based cartridge format** that embeds spritesheets and code
- **Built-in game selector** for managing multiple games
- **Cross-platform** support (Windows, Linux, macOS)

## Quick Start

### Prerequisites

#### Ubuntu/Debian
```bash
sudo apt-get install libsdl2-dev libsdl2-image-dev
```

#### macOS
```bash
brew install sdl2 sdl2_image
```

#### Windows
SDL2 development libraries are included in the repository.

### Building

1. Clone the repository with submodules:
```bash
git clone --recursive https://github.com/your-repo/TinyBit.git
cd TinyBit
```

2. If you forgot `--recursive`, initialize submodules:
```bash
git submodule init
git submodule update --recursive
```

3. Compile:
```bash
make
```

### Running

#### Launch Game Selector
```bash
./bin/tinybit
```

#### Play a Specific Game
```bash
./bin/tinybit games/flappy.tb.png
```

#### Create a Cartridge
```bash
./bin/tinybit -c spritesheet.png script.lua cover.png output.tb.png
```

## TinyBit API Reference

### System Specifications

- **Screen Resolution**: 128x128 pixels
- **Color Depth**: 4-bit RGBA (16 levels per channel)
- **Memory Layout**:
  - Spritesheet: 32KB (0x00000-0x07FFF)
  - Display: 32KB (0x08000-0x0FFFF)  
  - Script: 12KB (0x10000-0x12FFF)
  - User: 4KB (0x13000-0x13FFF)
- **Total Memory**: 80KB
- **Cartridge Size**: 200x230 pixels (PNG format)

### Input Controls

| TinyBit Button | Keyboard Key |
|---------------|--------------|
| A | A |
| B | B |
| UP | Arrow Up |
| DOWN | Arrow Down |
| LEFT | Arrow Left |
| RIGHT | Arrow Right |
| START | Enter |
| SELECT | Backspace |

### Lua API Functions

#### Graphics Functions

**Drawing Primitives**
- `cls()` - Clear the display
- `rect(x, y, w, h)` - Draw rectangle
- `oval(x, y, w, h)` - Draw oval
- `line(x1, y1, x2, y2)` - Draw line
- `sprite(sx, sy, sw, sh, dx, dy, dw, dh [, rotation])` - Draw sprite with optional rotation

**Polygon Drawing**
- `poly_add(x, y)` - Add vertex to polygon
- `poly_clear()` - Clear polygon vertices
- `draw_polygon()` - Draw filled polygon with current vertices

**Colors and Styles**
- `fill(r, g, b, a)` - Set fill color (0-255 per channel)
- `stroke(width, r, g, b, a)` - Set stroke color and width
- `text(r, g, b, a)` - Set text color

#### Text Functions
- `cursor(x, y)` - Set text cursor position
- `print(text)` - Print text at cursor position

#### Input Functions
- `btn(button)` - Check if button is currently held
- `btnp(button)` - Check if button was just pressed this frame

Button constants: `A`, `B`, `UP`, `DOWN`, `LEFT`, `RIGHT`, `START`, `SELECT`

#### Audio Functions (Placeholder)
- `tone(note, octave, duration, waveform [, volume, channel])` - Play musical tone
- `noise(duration [, volume, channel])` - Play white noise
- `bpm(beats_per_minute)` - Set tempo for timing

Audio constants:
- Notes: `C`, `Cs`/`Db`, `D`, `Ds`/`Eb`, `E`, `F`, `Fs`/`Gb`, `G`, `Gs`/`Ab`, `A`, `As`/`Bb`, `B`
- Waveforms: `SINE`, `SAW`, `SQUARE`

#### Memory Functions
- `peek(address)` - Read byte from memory
- `poke(address, value)` - Write byte to memory  
- `copy(dest, src, size)` - Copy memory region

#### Utility Functions
- `millis()` - Get current frame time in milliseconds
- `random(min, max)` - Generate random integer in range
- `log(message)` - Print debug message to console

#### Game Management (Selector Only)
- `gamecount()` - Get number of available games
- `gamecover(index)` - Load game cover for preview
- `gameload(index)` - Load and start game by index

### Game Structure

Every TinyBit game must implement a `_draw()` function that gets called every frame:

```lua
function _draw()
    -- Game logic and rendering code
    cls()  -- Clear screen
    
    if btnp(A) then
        -- Handle A button press
    end
    
    sprite(0, 0, 16, 16, 50, 50, 16, 16)  -- Draw sprite
end
```

### Example Game

```lua
-- Simple bouncing ball
x, y = 64, 64
dx, dy = 1, 1

function _draw()
    cls()
    
    -- Update position
    x = x + dx
    y = y + dy
    
    -- Bounce off walls
    if x <= 0 or x >= 120 then dx = -dx end
    if y <= 0 or y >= 120 then dy = -dy end
    
    -- Draw ball
    fill(255, 255, 255, 255)
    oval(x, y, 8, 8)
end
```

## Creating Cartridges

1. **Create Assets**: Design a 128x128 pixel spritesheet PNG
2. **Write Script**: Create a Lua file with your game logic
3. **Design Cover**: Create a 128x128 pixel cover image PNG
4. **Export Cartridge**: 
   ```bash
   ./bin/tinybit -c spritesheet.png script.lua cover.png game.tb.png
   ```

The cartridge format embeds the spritesheet and script data in the least significant bits of the cartridge PNG, while the cover image is overlaid on top for visual identification.

## Development

### Memory Layout
- **0x00000-0x07FFF**: Spritesheet data (128x128x4 bits)
- **0x08000-0x0FFFF**: Display buffer (128x128x8 bits) 
- **0x10000-0x12FFF**: Lua script storage
- **0x13000-0x13FFF**: User data/variables

### TinyBit Core API

The TinyBit system provides a C API for embedding:

```c
// Initialize system
void tinybit_init(struct TinyBitMemory* memory, uint8_t* button_state_ptr);

// Load cartridge data  
bool tinybit_feed_cartridge(uint8_t* cartridge_buffer, size_t bytes);

// Start game execution
bool tinybit_start();

// Main game loop
void tinybit_loop();

// Signal quit
void tinybit_quit();

// Set callback functions
void tinybit_render_cb(void (*render_func_ptr)());
void tinybit_poll_input_cb(void (*poll_input_func_ptr)());
void tinybit_sleep_cb(void (*sleep_func_ptr)(int ms));
void tinybit_get_ticks_ms_cb(int (*get_ticks_ms_func_ptr)());
void tinybit_log_cb(void (*log_func_ptr)(const char*));
void tinybit_gamecount_cb(int (*gamecount_func_ptr)());
void tinybit_gameload_cb(void (*gameload_func_ptr)(int index));
```

## Contributing

TinyBit is designed to be a simple, educational game development platform. Contributions are welcome for:

- Bug fixes and optimizations
- Additional Lua API functions
- Cross-platform improvements
- Example games and tutorials
- Documentation improvements
