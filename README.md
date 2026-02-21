# TinyBit Virtual Console

TinyBit is a virtual console for creating and playing simple 2D games using Lua scripting. Games are stored as PNG cartridge files that steganographically embed both spritesheet assets and Lua script data.

## Features

- **128x128 pixel display** with 4-bit RGBA color depth (RGBA4444)
- **Lua scripting** for game logic with built-in graphics, input, and audio APIs
- **PNG-based cartridge format** that embeds spritesheets and code in image LSBs
- **Audio engine** with ABC notation music and sound effects
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

3. Build with CMake:
```bash
mkdir build && cd build
cmake ..
cmake --build .
```

### Running

#### Launch Game Selector
```bash
./build/tinybit
```

#### Play a Specific Game
```bash
./build/tinybit games/flappy.tb.png
```

#### Create a Cartridge
```bash
./build/tinybit -c spritesheet.png script.lua cover.png output.tb.png
```

## System Specifications

- **Screen Resolution**: 128x128 pixels
- **Color Depth**: 4-bit RGBA (RGBA4444, 16 levels per channel)
- **Cartridge Size**: 200x230 pixels (PNG format)
- **Audio**: 22kHz 16-bit PCM, 2 channels (music + SFX)
- **Target Frame Rate**: 60 FPS

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

## Lua API Reference

### Graphics

- `cls()` - Clear the display
- `sprite(sx, sy, sw, sh, dx, dy, dw, dh [, rotation])` - Draw sprite with optional rotation
- `duplicate(sx, sy, sw, sh, dx, dy, dw, dh [, rotation])` - Copy display region
- `rect(x, y, w, h)` - Draw rectangle
- `oval(x, y, w, h)` - Draw oval
- `line(x1, y1, x2, y2)` - Draw line

#### Polygon Drawing
- `poly_add(x, y)` - Add vertex to polygon
- `poly_clear()` - Clear polygon vertices
- `draw_polygon()` - Draw filled polygon with current vertices

#### Colors and Styles

Colors are packed RGBA8888 values. Use the color helper functions to create them:

- `fill(color)` - Set fill color
- `stroke(width, color)` - Set stroke width and color
- `text(color)` - Set text color
- `rgba(r, g, b, a)` - Create color from RGBA (0-255 per channel)
- `rgb(r, g, b)` - Create color from RGB (alpha defaults to 255)
- `hsb(h, s, b)` - Create color from HSB (0-255 per component)
- `hsba(h, s, b, a)` - Create color from HSBA (0-255 per component)

### Text
- `cursor(x, y)` - Set text cursor position
- `print(text)` - Print text at cursor position

### Input
- `btn(button)` - Check if button is currently held
- `btnp(button)` - Check if button was just pressed this frame

Button constants: `A`, `B`, `UP`, `DOWN`, `LEFT`, `RIGHT`, `START`, `SELECT`

### Audio
- `music(abc_string)` - Play looping music from ABC notation
- `sfx(abc_string)` - Play one-shot sound effect from ABC notation
- `sfx_active()` - Check if a sound effect is currently playing
- `bpm(beats_per_minute)` - Set tempo

Waveform constants: `SINE`, `SAW`, `SQUARE`, `NOISE`

### Memory
- `peek(address)` - Read byte from user memory
- `poke(address, value)` - Write byte to user memory
- `copy(dest, src, size)` - Copy memory region

### Utilities
- `millis()` - Get current frame time in milliseconds
- `random(min, max)` - Generate random integer in range
- `log(message)` - Print debug message to console
- `sleep(ms)` - Delay execution

### Game Management (Selector Only)
- `gamecount()` - Get number of available games
- `gamecover(index)` - Load game cover for preview
- `gameload(index)` - Load and start game by index

### Global Constants
- `TB_SCREEN_WIDTH` (128), `TB_SCREEN_HEIGHT` (128)

## Game Structure

Every TinyBit game must implement a `_draw()` function that gets called every frame:

```lua
function _draw()
    cls()

    if btnp(A) then
        -- Handle A button press
    end

    sprite(0, 0, 16, 16, 50, 50, 16, 16)
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
    fill(rgb(255, 255, 255))
    oval(x, y, 8, 8)
end
```

## Creating Cartridges

1. **Create Assets**: Design a 128x128 pixel spritesheet PNG
2. **Write Script**: Create a Lua file with your game logic
3. **Design Cover**: Create a 128x128 pixel cover image PNG
4. **Export Cartridge**:
   ```bash
   ./build/tinybit -c spritesheet.png script.lua cover.png game.tb.png
   ```

The cartridge format embeds the spritesheet and script data in the least significant bits of the cartridge PNG, while the cover image is overlaid on top for visual identification.

## TinyBit Core C API

The TinyBit library can be embedded in any C project. See [src/tinybit/README.md](src/tinybit/README.md) for the full library documentation.

```c
// Initialize system
void tinybit_init(struct TinyBitMemory* memory);

// Load cartridge data
bool tinybit_feed_cartridge(const uint8_t* cartridge_buffer, size_t bytes);

// Game lifecycle
bool tinybit_start();
void tinybit_loop();
bool tinybit_restart();
void tinybit_stop();

// Platform callbacks
void tinybit_render_cb(void (*render_func_ptr)());
void tinybit_poll_input_cb(void (*poll_input_func_ptr)());
void tinybit_get_ticks_ms_cb(int (*get_ticks_ms_func_ptr)());
void tinybit_audio_queue_cb(void (*audio_queue_func_ptr)());
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
