#ifndef __CONSOLE_GRAPHICS_LOAD_H__
#define __CONSOLE_GRAPHICS_LOAD_H__

#include <libgimp/gimp.h>

#include "file-console-graphics-formatdefs.h"

gint32 load_image (const gchar *filename, 
                   const gchar *palette,
                   TileLoadFunc tile_func,
                   gboolean thumbnail,
                   GError **error);

void load_colormap_from_palette (const gchar *palette,
                                 guchar color_map[]);

gboolean load_dialog (const gchar *filename,
                      TileLoadFunc *func_out,
                      gchar **palette_out);

#endif /*__CONSOLE_GRAPHICS_LOAD_H__*/

