#include "../menu_driver.h"

#include "../../gfx/gfx_display.h"
#include "../../gfx/gfx_thumbnail.h"

#include "../../file_path_special.h"
#include "../../tasks/tasks_internal.h"

#include "../../paths.h"

#include "../../audio/audio_driver.h"

#include <retro_timers.h>

struct bitbox_handle {
   font_data_t *font;

   struct {
       bool update;
       char path[8204];
       gfx_thumbnail_t thumbnail;
   } thumbnail;

   gfx_thumbnail_t arrow;

   gfx_thumbnail_t systems[13];
};

typedef struct bitbox_handle bitbox_handle_t;

static void bitbox_update_savestate_thumbnail_path(void *data, unsigned i);
static void bitbox_update_savestate_thumbnail_image(void *data);

float selected_backgrounds[13][16] = {
    COLOR_HEX_TO_FLOAT(0xD35400, 1.0f), /* arcade */
    COLOR_HEX_TO_FLOAT(0x7F8C8D, 1.0f), /* gameboy */
    COLOR_HEX_TO_FLOAT(0x2C3E50, 1.0f), /* gamegear */
    COLOR_HEX_TO_FLOAT(0x8E44AD, 1.0f), /* gba */
    COLOR_HEX_TO_FLOAT(0x16A085, 1.0f), /* gbc */
    COLOR_HEX_TO_FLOAT(0x2969B0, 1.0f), /* megadrive */
    COLOR_HEX_TO_FLOAT(0x34495E, 1.0f), /* neogeo */
    COLOR_HEX_TO_FLOAT(0xD14841, 1.0f), /* nes */
    COLOR_HEX_TO_FLOAT(0xF1C40F, 1.0f), /* ngpx */
    COLOR_HEX_TO_FLOAT(0x212121, 1.0f), /* psx */
    COLOR_HEX_TO_FLOAT(0x1ABC9C, 1.0f), /* sms */
    COLOR_HEX_TO_FLOAT(0xFBC02D, 1.0f), /* snes */
    COLOR_HEX_TO_FLOAT(0xBDC3C7, 1.0f)  /* default */
};

static void bitbox_frame(
        void *data,
        video_frame_info_t *video_info) {
    gfx_display_t *p_disp   = disp_get_ptr();
    bitbox_handle_t *bitbox = (bitbox_handle_t *) data;
    size_t i, menu_start, menu_end, menu_selection;
    char platform[255];

    float background[16]            = COLOR_HEX_TO_FLOAT(0x000000, 0.5f);
    float border[16]                = COLOR_HEX_TO_FLOAT(0x000000, 1.0f);
    float thumbnail_background[16]  = COLOR_HEX_TO_FLOAT(0xffffff, 1.0f);
    float *selected_background;
    gfx_thumbnail_t *system;

    bitbox_update_savestate_thumbnail_path(data, (unsigned) menu_navigation_get_selection());
    bitbox_update_savestate_thumbnail_image(data);

    video_driver_set_viewport(video_info->width, video_info->height, true, false);

    /******************************/
    /* Change selected background */
    /* according to platform      */
    /******************************/
    const char *path = path_get(RARCH_PATH_CONTENT);

    if (string_starts_with(path, "/mnt/arcade/")) {
        selected_background = selected_backgrounds[0];
        system = &bitbox->systems[0];
    } else if (string_starts_with(path, "/mnt/gameboy/")) {
        selected_background = selected_backgrounds[1];
        system = &bitbox->systems[1];
    } else if (string_starts_with(path, "/mnt/gamegear/")) {
        selected_background = selected_backgrounds[2];
        system = &bitbox->systems[2];
    } else if (string_starts_with(path, "/mnt/gba/")) {
        selected_background = selected_backgrounds[3];
        system = &bitbox->systems[3];
    } else if (string_starts_with(path, "/mnt/gbc/")) {
        selected_background = selected_backgrounds[4];
        system = &bitbox->systems[4];
    } else if (string_starts_with(path, "/mnt/megadrive/")) {
        selected_background = selected_backgrounds[5];
        system = &bitbox->systems[5];
    } else if (string_starts_with(path, "/mnt/neogeo/")) {
        selected_background = selected_backgrounds[6];
        system = &bitbox->systems[6];
    } else if (string_starts_with(path, "/mnt/nes/")) {
        selected_background = selected_backgrounds[7];
        system = &bitbox->systems[7];
    } else if (string_starts_with(path, "/mnt/ngpx/")) {
        selected_background = selected_backgrounds[8];
        system = &bitbox->systems[8];
    } else if (string_starts_with(path, "/mnt/psx/")) {
        selected_background = selected_backgrounds[9];
        system = &bitbox->systems[9];
    } else if (string_starts_with(path, "/mnt/sms/")) {
        selected_background = selected_backgrounds[10];
        system = &bitbox->systems[10];
    } else if (string_starts_with(path, "/mnt/snes/")) {
        selected_background = selected_backgrounds[11];
        system = &bitbox->systems[11];
    } else {
        selected_background = selected_backgrounds[12];
        system = &bitbox->systems[12];
    }
    
    /*******************/
    /* Draw background */
    /*******************/
    gfx_display_draw_quad(
            p_disp,
            video_info->userdata,
            video_info->width,
            video_info->height,
            video_info->width * 0.75,
            0,
            video_info->width * 0.25,
            video_info->height,
            video_info->width,
            video_info->height,
            background,
            NULL);

    gfx_display_draw_quad(
            p_disp,
            video_info->userdata,
            video_info->width,
            video_info->height,
            video_info->width * 0.75 - 1,
            0,
            1,
            video_info->height,
            video_info->width,
            video_info->height,
            border,
            NULL);

    gfx_display_draw_quad(
            p_disp,
            video_info->userdata,
            video_info->width,
            video_info->height,
            video_info->width * 0.75,
            video_info->height * 0.75 - 30,
            video_info->width * 0.25,
            60,
            video_info->width,
            video_info->height,
            selected_background,
            NULL);

    /******************/
    /* Draw thumbnail */
    /******************/
    if (!string_is_empty(bitbox->thumbnail.path)) {
        float thumbnail_width, thumbnail_height;

        gfx_thumbnail_get_draw_dimensions(
                &bitbox->thumbnail.thumbnail,
                video_info->width * 0.2, video_info->width * 0.2, 1.0f,
                &thumbnail_width, &thumbnail_height);

        gfx_display_draw_quad(
                p_disp,
                video_info->userdata,
                video_info->width,
                video_info->height,
                10,
                10,
                12,
                12,
                video_info->width,
                video_info->height,
                border,
                NULL);

        gfx_display_draw_quad(
                p_disp,
                video_info->userdata,
                video_info->width,
                video_info->height,
                video_info->width * 0.75 - thumbnail_width - 20 - 5,
                video_info->height * 0.75 - thumbnail_height * 0.5 - 5,
                thumbnail_width + 10,
                thumbnail_height + 10,
                video_info->width,
                video_info->height,
                thumbnail_background,
                NULL);

        gfx_thumbnail_draw(
                video_info->userdata,
                video_info->width,
                video_info->height,
                &bitbox->thumbnail.thumbnail,
                video_info->width * 0.75 - thumbnail_width - 20,
                video_info->height * 0.75 - thumbnail_height * 0.5,
                thumbnail_width,
                thumbnail_height,
                GFX_THUMBNAIL_ALIGN_CENTRE,
                1.0f,
                1.0f,
                NULL);

        gfx_thumbnail_draw(
                video_info->userdata,
                video_info->width,
                video_info->height,
                &bitbox->arrow,
                video_info->width * 0.75 - 15,
                video_info->height * 0.75 - 12,
                12,
                24,
                GFX_THUMBNAIL_ALIGN_CENTRE,
                1.0f,
                1.0f,
                NULL);
    }

    /*****************/
    /* System corner */
    /*****************/
    gfx_thumbnail_draw(
            video_info->userdata,
            video_info->width,
            video_info->height,
            system,
            0,
            0,
            384,
            384,
            GFX_THUMBNAIL_ALIGN_CENTRE,
            1.0f,
            1.0f,
            NULL);

    /****************/
    /* Menu entries */
    /****************/
    menu_entries_ctl(MENU_ENTRIES_CTL_START_GET, &menu_start);
    menu_end = menu_entries_get_size();
    menu_selection = menu_navigation_get_selection();

    for (i = menu_start; i < menu_end; i++) {
        menu_entry_t entry;
        
        MENU_ENTRY_INITIALIZE(entry);
        entry.flags |= MENU_ENTRY_FLAG_RICH_LABEL_ENABLED
                     | MENU_ENTRY_FLAG_VALUE_ENABLED;
        menu_entry_get(&entry, 0, (unsigned) i, NULL, true);

        gfx_display_draw_text(
                bitbox->font,
                entry.path,
                video_info->width * 0.75 + 13.0f,
                video_info->height * 0.75 + font_driver_get_line_ascender(bitbox->font, 1.0f) * 0.5 + i * 60.0f - menu_selection * 60.0f,
                video_info->width,
                video_info->height,
                i == menu_selection ? 0x000000E0 : 0xFFFFFFFF,
                TEXT_ALIGN_LEFT,
                1.0f,
                false,
                1.0f,
                true);
        
        if (!string_is_equal(entry.value, "...") &&
                !string_is_equal(entry.value, "(PRESET)"))
            gfx_display_draw_text(
                    bitbox->font,
                    entry.value,
                    video_info->width - font_driver_get_message_width(bitbox->font, entry.value, strlen(entry.value), 1.0f) - 13.0f,
                    video_info->height * 0.75 + font_driver_get_line_ascender(bitbox->font, 1.0f) * 0.5 + i * 60.0f - menu_selection * 60.0f,
                    video_info->width,
                    video_info->height,
                    i == menu_selection ? 0x000000E0 : 0xFFFFFFFF,
                    TEXT_ALIGN_LEFT,
                    1.0f,
                    false,
                    1.0f,
                    true);
    }

    font_driver_flush(video_info->width, video_info->height, bitbox->font);
}

static void *bitbox_init(
        void **userdata,
        bool video_is_threaded) {
    menu_handle_t *menu     = NULL;
    bitbox_handle_t *bitbox = NULL;
    gfx_display_t *p_disp   = disp_get_ptr();

    if (!(menu = (menu_handle_t *) calloc(1, sizeof(*menu))))
        return NULL;

    if (!(bitbox = (bitbox_handle_t *) calloc(1, sizeof(*bitbox)))) {
        free(menu);
        return NULL;
    }

    *userdata = bitbox;

    /* Load fonts */
    bitbox->font = gfx_display_font_file(p_disp, "/usr/share/attract/layouts/BitBox/BebasNeue Bold.otf", 24, video_is_threaded);

    /* Load textures */
    gfx_thumbnail_request_file(
            "/usr/share/attract/layouts/BitBox/arrow.png",
            &bitbox->arrow,
            1.0f);

    gfx_thumbnail_request_file(
            "/usr/share/attract/layouts/BitBox/systems/arcade.png",
            &bitbox->systems[0],
            1.0f);

    gfx_thumbnail_request_file(
            "/usr/share/attract/layouts/BitBox/systems/gameboy.png",
            &bitbox->systems[1],
            1.0f);

    gfx_thumbnail_request_file(
            "/usr/share/attract/layouts/BitBox/systems/gamegear.png",
            &bitbox->systems[2],
            1.0f);

    gfx_thumbnail_request_file(
            "/usr/share/attract/layouts/BitBox/systems/gba.png",
            &bitbox->systems[3],
            1.0f);

    gfx_thumbnail_request_file(
            "/usr/share/attract/layouts/BitBox/systems/gbc.png",
            &bitbox->systems[4],
            1.0f);

    gfx_thumbnail_request_file(
            "/usr/share/attract/layouts/BitBox/systems/megadrive.png",
            &bitbox->systems[5],
            1.0f);

    gfx_thumbnail_request_file(
            "/usr/share/attract/layouts/BitBox/systems/neogeo.png",
            &bitbox->systems[6],
            1.0f);

    gfx_thumbnail_request_file(
            "/usr/share/attract/layouts/BitBox/systems/nes.png",
            &bitbox->systems[7],
            1.0f);

    gfx_thumbnail_request_file(
            "/usr/share/attract/layouts/BitBox/systems/ngpx.png",
            &bitbox->systems[8],
            1.0f);

    gfx_thumbnail_request_file(
            "/usr/share/attract/layouts/BitBox/systems/psx.png",
            &bitbox->systems[9],
            1.0f);

    gfx_thumbnail_request_file(
            "/usr/share/attract/layouts/BitBox/systems/sms.png",
            &bitbox->systems[10],
            1.0f);

    gfx_thumbnail_request_file(
            "/usr/share/attract/layouts/BitBox/systems/snes.png",
            &bitbox->systems[11],
            1.0f);

    gfx_thumbnail_request_file(
            "/usr/share/attract/layouts/BitBox/systems/default.png",
            &bitbox->systems[12],
            1.0f);

    gfx_display_init_white_texture();

    return menu;
}

static void bitbox_free(void *data) {
    bitbox_handle_t *bitbox = (bitbox_handle_t *) data;

    gfx_display_deinit_white_texture();
}

static void bitbox_update_savestate_thumbnail_path(void *data, unsigned i) {
    bitbox_handle_t *bitbox = (bitbox_handle_t *) data;
    menu_entry_t entry;
    settings_t *settings    = config_get_ptr();
    int state_slot          = settings->ints.state_slot;

    MENU_ENTRY_INITIALIZE(entry);
    entry.flags |= MENU_ENTRY_FLAG_LABEL_ENABLED;
    menu_entry_get(&entry, 0, i, NULL, true);
    
    if (!string_is_empty(entry.label)) {
        if (string_to_unsigned(entry.label) == MENU_ENUM_LABEL_STATE_SLOT ||
                string_is_equal(entry.label, "state_slot") ||
                string_is_equal(entry.label, "loadstate") ||
                string_is_equal(entry.label, "savestate")) {
            char path[8204];
            runloop_state_t *runloop_st = runloop_state_get_ptr();

            if (string_to_unsigned(entry.label) == MENU_ENUM_LABEL_STATE_SLOT) {
                state_slot = i - 1;
            }

            if (state_slot < 0) {
                path[0] = '\0';
                fill_pathname_join_delim(path, runloop_st->name.savestate, "auto", '.', sizeof(path));
            } else {
                size_t _len = strlcpy(path, runloop_st->name.savestate, sizeof(path));
                if (state_slot > 0)
                    snprintf(path + _len, sizeof(path) - _len, "%d", state_slot);
            }

            strlcat(path, FILE_PATH_PNG_EXTENSION, sizeof(path));

            if (path_is_valid(path)) {
               if ( !string_is_equal(path, bitbox->thumbnail.path)) {
                   strlcpy(bitbox->thumbnail.path, path, sizeof(bitbox->thumbnail.path));
                   bitbox->thumbnail.update = true;
               }
            } else {
                bitbox->thumbnail.path[0] = '\0';
                bitbox->thumbnail.update = true;
            }
        } else {
            bitbox->thumbnail.path[0] = '\0';
            bitbox->thumbnail.update = true;
        }
    }
}

static void bitbox_update_savestate_thumbnail_image(void *data) {
    bitbox_handle_t *bitbox = (bitbox_handle_t *) data;

    if (bitbox->thumbnail.update) {
        gfx_thumbnail_request_file(
                bitbox->thumbnail.path,
                &bitbox->thumbnail.thumbnail,
                1.0f);

        bitbox->thumbnail.update = false;
    }
}

static int bitbox_menu_entry_action(
        void *userdata,
        menu_entry_t *entry,
        size_t i,
        enum menu_action action) {

    /* Going back from quick menu is forbidden */
    if (action == MENU_ACTION_CANCEL && menu_is_running_quick_menu())
        return generic_menu_entry_action(userdata, entry, i, MENU_ACTION_SELECT);

    /* Play sound when navigation */
    if (action == MENU_ACTION_UP ||
            action == MENU_ACTION_DOWN ||
            action == MENU_ACTION_LEFT ||
            action == MENU_ACTION_RIGHT ||
            action == MENU_ACTION_CANCEL) {
        audio_driver_mixer_play_menu_sound(AUDIO_MIXER_SYSTEM_SLOT_OK);
    }

    return generic_menu_entry_action(userdata, entry, i, action);
}

menu_ctx_driver_t menu_ctx_bitbox = {
   NULL, 
   NULL, 
   NULL, 
   bitbox_frame, 
   bitbox_init,
   bitbox_free,
   NULL, 
   NULL, 
   NULL, 
   NULL,
   NULL, 
   NULL,
   NULL,
   NULL, 
   NULL, 
   NULL, 
   NULL, 
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   "bitbox",
   NULL, 
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   bitbox_update_savestate_thumbnail_path,
   bitbox_update_savestate_thumbnail_image,
   NULL,
   NULL, 
   bitbox_menu_entry_action
};
