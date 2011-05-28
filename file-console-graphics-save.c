/*
 * GIMP SNES File Plugin
 * Copyright (C) 2011 Cheeseum
 
 * 
 */

#include <errno.h>
#include <glib/gstdio.h>

#include <libgimp/gimp.h>
#include <libgimp/gimpui.h>

#include "file-console-graphics.h"
#include "file-console-graphics-save.h"
#include "file-console-graphics-formatdefs.h"

/* tile is 32 bits */
#define WriteTileOK(file,buffer) (fwrite(buffer, 32, 1, file) == 1)

gboolean save_image (const gchar *filename, 
                     gint32 image_ID,
                     gint32 drawable_ID,
                     gint32 orig_image_ID,
                     TileSaveFunc save_func,
                     GError **error)
{
    FILE *outfile;
    guchar buf[64];
    guchar dest[32];
    
    gint colors;
    
    gint row, col;
    gint rows, cols; /* of 8x8 tiles in the image */
    
    GimpPixelRgn pixel_rgn;
    GimpDrawable *drawable;
    GimpImageType drawable_type;
    
    drawable = gimp_drawable_get (drawable_ID);
    drawable_type = gimp_drawable_type (drawable_ID);

    if (drawable_type == GIMP_RGB_IMAGE)
    {
        g_message("Cannot save RGB color images. Convert to "
                  "indexed color or grayscale first.\n");
        return FALSE;
    }

    gimp_image_get_colormap (image_ID, &colors);
    if (colors > 16)
    {
        g_message("Cannot save Indexed color images with more than "
                  "16 colors. Reduce number of colors first.\n");
        return FALSE;
    }

    outfile = g_fopen (filename, "wb");
    if (!outfile)
    {
        g_set_error (error, G_FILE_ERROR, g_file_error_from_errno (errno),
                     "Could not open '%s' for writing: %s\n",
                     gimp_filename_to_utf8 (filename), g_strerror (errno));
        return FALSE;
    }

    gimp_progress_init_printf ("Saving '%s'",
                               gimp_filename_to_utf8 (filename));
    
    gimp_pixel_rgn_init (&pixel_rgn, drawable, 0, 0,
                         drawable->width, drawable->height, FALSE, FALSE);

    rows = drawable->height / 8;
    cols = drawable->width / 8;

    for (row = 0; row < rows; row++)
    {
        for (col = 0; col < cols; col++)
        {
            gimp_pixel_rgn_get_rect (&pixel_rgn, buf,
                                     col * 8, row * 8, 8, 8);
            
            save_func ((const guchar *)buf, dest);

            if (!WriteTileOK(outfile, dest))
            {
                g_message("Error writing to file '%s'\n",
                          gimp_filename_to_utf8 (filename));
                return FALSE;
            }
        }
    }

    gimp_drawable_detach (drawable);
    return TRUE;
}

GimpPDBStatusType sanity_check (const gchar *filename,
                                gint32 image_ID,
                                GimpRunMode run_mode,
                                GError **error)
{
    /* Check for proper dimensions */
    guint image_width, image_height, wrem, hrem;
    
    image_width = gimp_image_width (image_ID);
    image_height = gimp_image_height (image_ID);
    wrem = image_width % 8;
    hrem = image_height % 8;
    
    if (wrem != 0 || hrem != 0)
    {
        if ((run_mode == GIMP_RUN_NONINTERACTIVE) || bad_bounds_dialog ())
        {
            gimp_image_crop   (image_ID, 
                               image_width - wrem, image_height - hrem,
                               0, 0);
            return GIMP_PDB_SUCCESS;
        }
        else 
        {
            return GIMP_PDB_CANCEL;
        }
    }
}

gboolean save_dialog (const gchar *filename,
                      TileSaveFunc *func_out)
{
    GtkWidget *dialog, *format_sel;
    gboolean save;

    dialog = gimp_dialog_new("Save Tiled Console Graphics Image", PLUG_IN_ROLE,
                             NULL, 0, gimp_standard_help_func, SAVE_PROC,
                             GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                             "Save", GTK_RESPONSE_OK, NULL);

    /* FIXME: make this pull values right from FormatList */
    format_sel = gtk_combo_box_text_new ();
    gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT(format_sel), "SNES 4BPP");
    gtk_combo_box_set_active (GTK_COMBO_BOX(format_sel), 0);

    gtk_box_pack_end (GTK_BOX (gtk_dialog_get_content_area (GTK_DIALOG (dialog))),
                      format_sel, TRUE, TRUE, 0);
    
    gimp_window_set_transient (GTK_WINDOW (dialog));
    
    gtk_widget_show (format_sel);
    gtk_widget_show (dialog);

    save = (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_OK);

    if (save)
    {
        gint fmtidx = format_def_from_string ((const gchar *)gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(format_sel)));
        
        if (fmtidx >= 0)
            *func_out = FormatList[fmtidx].save_func;
    } 
    
    gtk_widget_destroy (dialog);
    
    return save;
}

gboolean
bad_bounds_dialog (void)
{
    GtkWidget *dialog;
    gboolean   crop;

    dialog = gtk_message_dialog_new (NULL, 0,
                                   GTK_MESSAGE_WARNING, GTK_BUTTONS_NONE,
                                   "The image you are trying to save as a "
                                   "console graphics image has dimensions which "
                                   "are not a multiple of 8.");

    gtk_dialog_add_buttons (GTK_DIALOG (dialog),
                          GTK_STOCK_CANCEL,     GTK_RESPONSE_CANCEL,
                          GIMP_STOCK_TOOL_CROP, GTK_RESPONSE_OK,
                          NULL);

    gtk_dialog_set_alternative_button_order (GTK_DIALOG (dialog),
                                           GTK_RESPONSE_OK,
                                           GTK_RESPONSE_CANCEL,
                                           -1);

    gimp_window_set_transient (GTK_WINDOW (dialog));

    gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (dialog),
                                            "This file format does not "
                                            "allow this.  You may choose "
                                            "whether to crop all of the "
                                            "layers to the image borders, "
                                            "or cancel this save.");

    gtk_widget_show (dialog);

    crop = (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_OK);

    gtk_widget_destroy (dialog);

    return crop;
}
