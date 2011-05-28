#include <libgimp/gimp.h>
#include "file-console-graphics-formatdefs.h"

/* while this is last in the header for sensibility,
 * it is also first in the source for sensibility 
 */

gint format_def_from_string (const gchar *string) {
    gint length = sizeof(FormatList)/sizeof(Formats);
    gint i;

    for (i = 0; i < length; i++)
    {
        if (strcmp (string, FormatList[i].key) == 0)
            return i;
    }
}

/* Tile Loading and Saving Funcs
 * each will load or save data from guchar *src
 * in or from specified format to *dest
 *
 * src must contain enough data (8 x num_bitplanes) / (8x8)
 * dest must be large enough to contain the tile (8 x 8) / (8 x num_bitplanes)
 */

/* SNES 4BPP */
void load_snes_4bpp_tile (const guchar src[], guchar dest[])
{
    guint d = 0;
    gint i, j;
    for (i = 0; i < 16; i += 2)
    {
        guchar bp1 = src[i];
        guchar bp2 = src[i+1];
        guchar bp3 = src[i+16];
        guchar bp4 = src[i+17];
        
        for (j = 0; j < 8; j++)
        {
            guchar color = 0;
            if (bp1 & (0x80 >> j))
                color |= 1;
            if (bp2 & (0x80 >> j))
                color |= 2;
            if (bp3 & (0x80 >> j))
                color |= 4;
            if (bp4 & (0x80 >> j))
                color |= 8;

            dest[d++] = color;
        }
    }
}

void save_snes_4bpp_tile (const guchar src[], guchar dest[])
{
    guint pix = 0;
    gint i, j;
    for (i = 0; i < 16; i+=2)
    {
        guchar bp1, bp2, bp3, bp4;
        bp1 = bp2 = bp3 = bp4 = 0;
        for (j = 0; j < 8; j++)
        {
            if (src[pix] & 1)
                bp1 |= (0x80 >> j);
            if (src[pix] & 2)
                bp2 |= (0x80 >> j);
            if (src[pix] & 4)
                bp3 |= (0x80 >> j);
            if (src[pix] & 8)
                bp4 |= (0x80 >> j);
            pix++;
        }
        dest[i]    = bp1;
        dest[i+1]  = bp2;
        dest[i+16] = bp3;
        dest[i+17] = bp4;
    }
}
