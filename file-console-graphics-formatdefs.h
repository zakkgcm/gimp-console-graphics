#ifndef __CONSOLE_GRAPHICS_FORMATDEFS_H__
#define __CONSOLE_GRAPHICS_FORMATDEFS_H__

typedef void(*TileLoadFunc)(const guchar src[], guchar dest[]);
typedef void(*TileSaveFunc)(const guchar src[], guchar dest[]);

void load_snes_4bpp_tile (const guchar src[], guchar dest[]);
void save_snes_4bpp_tile (const guchar src[], guchar dest[]);

typedef struct _Formats
{
    const gchar *key;
    TileLoadFunc load_func;
    TileSaveFunc save_func;
} Formats;

static Formats FormatList[] = 
{
    { "SNES 4BPP", &load_snes_4bpp_tile, &save_snes_4bpp_tile },
};
gint format_def_from_string (const gchar *string);

#endif /*__CONSOLE_GRAPHICS_FORMATDEFS_H__*/
