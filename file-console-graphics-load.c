#include <errno.h>
#include <string.h>
#include <glib/gstdio.h>

#include <libgimp/gimp.h>
#include <libgimp/gimpui.h>

#include "file-console-graphics.h"
#include "file-console-graphics-load.h"
#include "file-console-graphics-formatdefs.h"

/*typedef void(*TileLoadFunc)(const guchar src[], guchar dest[]);

static void load_snes_4bpp_tile (const guchar src[], guchar dest[]);*/


/* tile is 32 bits */
#define ReadTileOK(file,buffer) (fread(buffer, 32, 1, file) != 0)

/* FIXME: thumbnail doesn't do anything */
gint32 load_image (const gchar *filename,
                   const gchar *palette,
                   TileLoadFunc load_func,
                   gboolean thumbnail,
                   GError **error)
{
    static gint32 image_ID = -1;

    FILE *fd;
    guchar buf[32];
    guchar color_map[48];
    
    gint row, col = 0;
    gint filesize, width, height;
    
    gint32          layer_ID;
    GimpPixelRgn    pixel_rgn;
    GimpDrawable    *drawable;

    fd = g_fopen (filename, "rb");
    if (! fd)
    {
        g_set_error (error, G_FILE_ERROR, g_file_error_from_errno (errno), 
                    "Could not open '%s' for reading: %s",
                    gimp_filename_to_utf8 (filename), g_strerror(errno));
        return -1;
    }
    
    gimp_progress_init_printf ("Opening '%s'",
                               gimp_filename_to_utf8 (filename));
    

    fseek (fd, 0L, SEEK_END);
    filesize = ftell (fd);
    width = (filesize >= 512) ? 128 : (filesize / 32) * 8;
    height = MAX(1, (filesize / 512)) * 8; /* tiles in a row (16*32) times 8px per tile */
    fseek (fd, 0L, SEEK_SET);
    
    image_ID = gimp_image_new (width, height, GIMP_INDEXED);
    gimp_image_set_filename (image_ID, filename);

    /* console graphics don't have embedded palettes, we have to use gimp's */
    load_colormap_from_palette (palette, color_map);
    gimp_image_set_colormap (image_ID, color_map, 16);

    layer_ID = gimp_layer_new (image_ID,
                               "Background",
                               width, height,
                               GIMP_INDEXED_IMAGE,
                               100, GIMP_NORMAL_MODE);
    
    gimp_image_add_layer (image_ID, layer_ID, 0);
    
    drawable = gimp_drawable_get (layer_ID);

    gimp_pixel_rgn_init (&pixel_rgn, drawable,
                             0, 0, drawable->width, drawable->height, TRUE, FALSE);

    while (TRUE)
    {
        if (! ReadTileOK (fd, &buf))
        {
            /* no more tiles to read, not an error */
            return image_ID;
        }

        guchar dest[64];
        load_func ((const guchar *)buf, dest);
        
        /*
        guint d = 0;
        gint i, j;
        for (i = 0; i < 16; i += 2) {
            guchar bp1 = buf[i];
            guchar bp2 = buf[i+1];
            guchar bp3 = buf[i+16];
            guchar bp4 = buf[i+17];
            
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
        */
        gimp_pixel_rgn_set_rect (&pixel_rgn, dest,
                                 col * 8, row * 8, 8, 8);

        if (col == 15) 
        {
            row++;
            col = 0;
        } 
        else 
        {
            col++;
        }
    }
    
    gimp_drawable_flush (drawable);
    gimp_drawable_detach (drawable);

    return image_ID;
}

/* converts a gimp palette of given name to a colormap */
void load_colormap_from_palette (const gchar *palette,
                                 guchar color_map[])
{
    gint num_colors;
    GimpRGB *colors = gimp_palette_get_colors(palette, &num_colors);
    
    int i, j = 0;
    for (i = 0; i < 16; i++)
    {
        guchar red, green, blue;
        
        gimp_rgb_get_uchar (&colors[i], &red, &green, &blue);
        color_map[j++] = red;
        color_map[j++] = green;
        color_map[j++] = blue;
    }
}

gboolean load_dialog (const gchar *filename,
                      TileLoadFunc *func_out,
                      gchar **palette_out)
{
    GtkWidget *dialog, *palette_sel, *format_sel;
    const gchar *palette;
    gboolean load;

    dialog = gimp_dialog_new("Import Tiled Console Graphics Image", PLUG_IN_ROLE,
                             NULL, 0, gimp_standard_help_func, LOAD_PROC,
                             GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                             "Import", GTK_RESPONSE_OK, NULL);
    palette_sel = gimp_palette_select_button_new (NULL, "Default");

    format_sel = gtk_combo_box_text_new ();
    gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT(format_sel), "SNES 4BPP");
    gtk_combo_box_set_active (GTK_COMBO_BOX(format_sel), 0);

    gtk_box_pack_start (GTK_BOX (gtk_dialog_get_content_area (GTK_DIALOG (dialog))),
                        palette_sel, TRUE, TRUE, 0);
    
    gtk_box_pack_end (GTK_BOX (gtk_dialog_get_content_area (GTK_DIALOG (dialog))),
                      format_sel, TRUE, TRUE, 0);

    gimp_window_set_transient (GTK_WINDOW (dialog));
    
    gtk_widget_show (format_sel);
    gtk_widget_show (palette_sel);
    gtk_widget_show (dialog);

    load = (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_OK);

    if (load)
    {
        palette = gimp_palette_select_button_get_palette (GIMP_PALETTE_SELECT_BUTTON(palette_sel));
        *palette_out = g_malloc(sizeof(gchar) * strlen(palette));
        strcpy(*palette_out, palette);

        gint fmtidx = format_def_from_string ((const gchar *)gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(format_sel)));
        
        if (fmtidx >= 0)
            *func_out = FormatList[fmtidx].load_func;
    } 
    gtk_widget_destroy (dialog);

    return load;
}

/* Tile Loading Funcs
 * each will load data from guchar *src
 * in specified format to *dest
 *
 * src must contain enough data (8 x num_bitplanes)
 * dest must be large enough to contain the tile (8 x 8)
 */

/* SNES 4BPP */
/*static void load_snes_4bpp_tile (const guchar src[], guchar dest[])
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
}*/


