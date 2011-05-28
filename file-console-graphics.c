/*
 * GIMP SNES File Plugin
 * Copyright (C) 2011 Cheeseum
 
 * 
 */

#include <errno.h>
#include <string.h>
#include <glib/gstdio.h>

#include <libgimp/gimp.h>
#include <libgimp/gimpui.h>

#include "file-console-graphics.h"
#include "file-console-graphics-load.h"
#include "file-console-graphics-save.h"
#include "file-console-graphics-formatdefs.h"

/* Declare some local functions. */
static void query();
static void run(const gchar* name, gint nparams, const GimpParam* param, gint* nreturn_vals, GimpParam** return_vals);

static GimpRunMode run_mode;

const GimpPlugInInfo PLUG_IN_INFO =
{
	NULL,  /* init_proc  */
	NULL,  /* quit_proc  */
	(GimpQueryProc)query,
	(GimpRunProc)run,
};

MAIN ()

static void query()
{
	static const GimpParamDef load_args[] =
	{
		{ GIMP_PDB_INT32,	"run-mode",		"Interactive, non-interactive" },
		{ GIMP_PDB_STRING,	"filename",		"The name of the file to load" },
		{ GIMP_PDB_STRING,	"raw-filename",	"The name of the file to load" }
	};
	static const GimpParamDef load_return_vals[] =
	{
		{ GIMP_PDB_IMAGE,   "image",         "Output image" }
	};
	
    static const GimpParamDef save_args[] =
	{
		{ GIMP_PDB_INT32,	 "run-mode",		"Interactive, non-interactive" },
        { GIMP_PDB_IMAGE,    "image",           "Image to save" },
        { GIMP_PDB_DRAWABLE, "drawable",        "Drawable to save" },
		{ GIMP_PDB_STRING,	 "filename",		"The name of the file to load" },
		{ GIMP_PDB_STRING,	 "raw-filename",	"The name of the file to load" }
	};

    gimp_install_procedure (LOAD_PROC,
                            "Loads tiled console graphics files.",
                            "Reads tile-based console graphics files (gfx) using a gimp color palette.",
                            "Cheeseum",
                            "Cheeseum",
                            "2011",
                            "Tiled Console Graphics",
                            NULL,
                            GIMP_PLUGIN,
                            G_N_ELEMENTS (load_args),
                            G_N_ELEMENTS (load_return_vals),
                            load_args, load_return_vals);

    gimp_install_procedure (SAVE_PROC,
                            "Saves files in tiled console graphics format.",
                            "FIXME: Write help for console graphics loading plug-in.",
                            "Cheeseum",
                            "Cheeseum",
                            "2011",
                            "Tiled Console Graphics",
                            "INDEXED*, GRAY*",
                            GIMP_PLUGIN,
                            G_N_ELEMENTS (save_args), 0,
                            save_args, NULL);

    gimp_register_load_handler (LOAD_PROC, "bin,chr","");
    gimp_register_save_handler (SAVE_PROC, "bin,chr", "");
}

static void run (const gchar *name,
                 gint nparams,
                 const GimpParam *param,
                 gint *nreturn_vals,
                 GimpParam **return_vals)
{
    static GimpParam values[4];
    GimpPDBStatusType  status = GIMP_PDB_SUCCESS;
    GError            *error  = NULL;

    run_mode = param[0].data.d_int32;
    
    *nreturn_vals = 1;
    *return_vals = values;

    values[0].type              = GIMP_PDB_STATUS;
    values[0].data.d_status     = GIMP_PDB_EXECUTION_ERROR;

    if (strcmp (name, LOAD_PROC) == 0)
    {
        const gchar *filename;
        TileLoadFunc load_func;
        gchar *palette;
        gint32 image_ID;

        filename = param[1].data.d_string;
        
        switch (run_mode)
        {
            case GIMP_RUN_INTERACTIVE:
            case GIMP_RUN_WITH_LAST_VALS:
                gimp_ui_init (PLUG_IN_BINARY, FALSE);

                if (!load_dialog (filename, &load_func, &palette))
                    status = GIMP_PDB_CANCEL;

                break;

            case GIMP_RUN_NONINTERACTIVE:
                /* FIXME: implement non-interactive mode */
                status = GIMP_PDB_EXECUTION_ERROR;
                break;
        }

        if (status == GIMP_PDB_SUCCESS)
        {
            image_ID = load_image (filename, (const gchar *)palette, load_func, FALSE, &error);
            g_free(palette);
            
            if (image_ID != -1) {
                *nreturn_vals = 2;
                values[1].type         = GIMP_PDB_IMAGE;
                values[1].data.d_image = image_ID;
            }
            else
            {
                status = GIMP_PDB_EXECUTION_ERROR;
            }
        }
    }

    else if (strcmp (name, LOAD_THUMB_PROC) == 0)
    {
        /* FIXME: implement thumbnail proc */
        status = GIMP_PDB_CALLING_ERROR;
        //image_ID = load_image (filename, "Default", TRUE, &error);
    }

    else if (strcmp (name, SAVE_PROC) == 0)
    {
        const gchar *filename;
        TileSaveFunc save_func;

        gint32 image_ID;
        gint32 orig_image_ID;
        gint32 drawable_ID;
    
        GimpExportReturn export = GIMP_EXPORT_CANCEL;
        
        image_ID    = orig_image_ID = param[1].data.d_int32;
        drawable_ID = param[2].data.d_int32;
        filename    = param[3].data.d_string;
        
        switch (run_mode)
        {
            case GIMP_RUN_INTERACTIVE:
            case GIMP_RUN_WITH_LAST_VALS:
                {
                    gimp_ui_init (PLUG_IN_BINARY, FALSE);
                    status = sanity_check (filename, image_ID, run_mode, &error);

                    /* FIXME: warn the user that converting to
                     * Indexed from RGB might screw up palletes */

                    GimpExportCapabilities capabilities = 
                        GIMP_EXPORT_CAN_HANDLE_INDEXED |
                        GIMP_EXPORT_CAN_HANDLE_GRAY;

                    export = gimp_export_image (&image_ID, &drawable_ID, 
                                                "Console Graphics", capabilities);
                    if (export == GIMP_EXPORT_CANCEL)
                    {
                        status = GIMP_PDB_CANCEL;
                        break;
                    }
                    
                    if (!save_dialog (filename, &save_func))
                        status = GIMP_PDB_CANCEL;
                }
                break;
            case GIMP_RUN_NONINTERACTIVE:
                /* FIXME: implement non-interactive mode */
                status = GIMP_PDB_EXECUTION_ERROR;
                break;
            default:
                break;
        }
        
        /* Write the image to file */
        if (status = GIMP_PDB_SUCCESS) 
        {
            if (!save_image (param[3].data.d_string, 
                             image_ID, drawable_ID, orig_image_ID,
                             FormatList[0].save_func, &error))
            {
                status = GIMP_PDB_EXECUTION_ERROR;
            }
        }

        if (export == GIMP_EXPORT_EXPORT)
            gimp_image_delete (image_ID);
    }

    if (status != GIMP_PDB_SUCCESS && error)
    {
        *nreturn_vals = 2;
        values[1].type          = GIMP_PDB_STRING;
        values[1].data.d_string = error->message;
    }

    values[0].data.d_status = status;
}
