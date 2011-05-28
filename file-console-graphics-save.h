#include <libgimp/gimp.h>

#include "file-console-graphics-formatdefs.h"

GimpPDBStatusType sanity_check (const gchar *filename,
                                gint32 image_ID,
                                GimpRunMode run_mode,
                                GError **error);

gboolean save_dialog (const gchar *filename,
                      TileSaveFunc *func_out);

gboolean bad_bounds_dialog (void);

gint32 save_image (const gchar *filename, 
                   gint32 image_ID,
                   gint32 drawable_ID,
                   gint32 orig_image_ID,
                   TileSaveFunc save_func,
                   GError **error);
