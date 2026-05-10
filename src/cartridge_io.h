#ifndef CARTRIDGE_IO_H
#define CARTRIDGE_IO_H

#include <stdint.h>

struct CartridgeHeaderOpts {
    uint16_t format_version;
    uint16_t flags;
    const char* title;
    const char* author;
    uint16_t game_version;
    uint32_t package_date;
};

void load_game(char* path);
void export_cartridge(const char* sprite, const char* script, const char* cover,
                      const char* path, const struct CartridgeHeaderOpts* opts);

#endif
