#include "runesos_app.h"
#include "theme_nordico.h"
#include "hal_runesos.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

extern const runesos_hal_t runesos_hal_mock;

typedef enum {
    ENTRY_DIR,
    ENTRY_FILE_TXT,
    ENTRY_FILE_CFG,
    ENTRY_FILE_MEDIA,
    ENTRY_FILE_IMG
} file_type_t;

typedef struct {
    const char  *name;
    const char  *parent_dir;
    file_type_t  type;
    uint32_t     size_bytes;
    const char  *content;
} vfs_node_t;

/* Estrutura de arquivos virtuais para navegação */
static const vfs_node_t s_fs_nodes[] = {
    /* Raiz */
    {"notes",   "/sdcard", ENTRY_DIR, 0, NULL},
    {"system",  "/sdcard", ENTRY_DIR, 0, NULL},
    {"media",   "/sdcard", ENTRY_DIR, 0, NULL},
    {"logs",    "/sdcard", ENTRY_DIR, 0, NULL},
    {"home",    "/sdcard", ENTRY_DIR, 0, NULL},
    {"README.txt", "/sdcard", ENTRY_FILE_TXT, 1420,
        "=== RunesOS File System ===\n"
        "Bem-vindo ao armazenamento do seu cyberdeck.\n"
        "Aqui voce encontra arquivos do sistema, notas e logs.\n\n"
        "Formato: LittleFS (Flash 16MB) + FatFS (MicroSD).\n"},
    {"version.txt", "/sdcard", ENTRY_FILE_TXT, 256,
        "RunesOS Versao: 0.2-alpha\n"
        "Kernel: FreeRTOS v10.5\n"
        "GUI: LVGL 9.x\n"
        "Target: ESP32-S3 Barkley's Tin\n"},

    /* Pasta notes */
    {"ideias.txt", "/sdcard/notes", ENTRY_FILE_TXT, 512,
        "# Ideias de Projetos Runicos\n"
        "- Criar leitor de RFID para cartas rúnicas\n"
        "- Implementar sintetizador de voz com acento nordico\n"
        "- Conectar sensor de pressao barometrica BMP280\n"},
    {"runas_significados.md", "/sdcard/notes", ENTRY_FILE_TXT, 1890,
        "# Elder Futhark - Guia Rapido\n\n"
        "ᚠ Fehu: Abundancia e ouro\n"
        "ᚢ Uruz: Forca indomavel\n"
        "ᚦ Thurisaz: Espinho defensivo\n"
        "ᚨ Ansuz: Soprado pelos deuses\n"
        "ᚱ Raidho: Jornadas e caminhos\n"},
    {"senhas_mock.cfg", "/sdcard/notes", ENTRY_FILE_CFG, 128,
        "[WIFI_SECRETS]\n"
        "Home_SSID = Valhalla_5G\n"
        "Work_SSID = Asgard_IoT\n"},

    /* Pasta system */
    {"runesos.cfg", "/sdcard/system", ENTRY_FILE_CFG, 340,
        "[System]\n"
        "display_brightness = 80\n"
        "buzzer_volume = 60\n"
        "theme = nordico_gold\n"
        "auto_sleep_sec = 60\n"},
    {"display.ini", "/sdcard/system", ENTRY_FILE_CFG, 220,
        "[ST7701]\n"
        "width = 480\n"
        "height = 640\n"
        "driver = RGB_PARALLEL\n"
        "touch_ic = GT911_I2C\n"},
    {"wifi.json", "/sdcard/system", ENTRY_FILE_CFG, 180,
        "{\n  \"autoconnect\": true,\n  \"dhcp\": true,\n  \"ntp_server\": \"pool.ntp.org\"\n}\n"},

    /* Pasta home do usuario */
    {"user",      "/sdcard/home", ENTRY_DIR, 0, NULL},
    {"Desktop",   "/sdcard/home/user", ENTRY_DIR, 0, NULL},
    {"Documents", "/sdcard/home/user", ENTRY_DIR, 0, NULL},
    {"Downloads", "/sdcard/home/user", ENTRY_DIR, 0, NULL},
    {"Music",     "/sdcard/home/user", ENTRY_DIR, 0, NULL},
    {"Pictures",  "/sdcard/home/user", ENTRY_DIR, 0, NULL},
    {"Videos",    "/sdcard/home/user", ENTRY_DIR, 0, NULL},

    /* Pasta media */
    {"hymn_of_norse.mp3", "/sdcard/media", ENTRY_FILE_MEDIA, 3450000, NULL},
    {"nordic_wallpaper.png", "/sdcard/media", ENTRY_FILE_IMG, 185000, NULL},
    {"runes_font.ttf", "/sdcard/media", ENTRY_FILE_CFG, 45000, NULL},

    /* Pasta logs */
    {"boot.log", "/sdcard/logs", ENTRY_FILE_TXT, 1200,
        "[0.000] Bootloader inicializado (ESP32-S3 @ 240MHz)\n"
        "[0.120] 8MB PSRAM detectada e mapeada\n"
        "[0.240] Display ST7701 480x640 inicializado\n"
        "[0.310] Touch GT911 detectado em I2C 0x5D\n"
        "[0.400] Montando SD Card... OK (16 GB FAT32)\n"
        "[0.550] RunesOS GUI pronta.\n"},
    {"dmesg.txt", "/sdcard/logs", ENTRY_FILE_TXT, 850,
        "FreeRTOS: Scheduler ativo no Core 0 e Core 1\n"
        "Heap livre: 7854320 bytes\n"
        "Bateria: 3850 mV (87%)\n"}
};

#define TOTAL_FS_NODES (sizeof(s_fs_nodes) / sizeof(s_fs_nodes[0]))

static char s_current_dir[64] = "/sdcard";
static lv_obj_t *s_parent_container = NULL;
static lv_obj_t *s_browser_view = NULL;
static lv_obj_t *s_viewer_view = NULL;
static lv_obj_t *s_path_label = NULL;

static void render_browser(void);
static void show_file_viewer(const vfs_node_t *node);

static const char *get_icon_for_type(file_type_t type)
{
    switch (type) {
        case ENTRY_DIR:        return "📁";
        case ENTRY_FILE_TXT:   return "📄";
        case ENTRY_FILE_CFG:   return "⚙️";
        case ENTRY_FILE_MEDIA: return "🎵";
        case ENTRY_FILE_IMG:   return "🖼️";
        default:               return "📄";
    }
}

static void node_clicked_cb(lv_event_t *e)
{
    const vfs_node_t *node = (const vfs_node_t *)lv_event_get_user_data(e);
    if (!node) return;

    if (node->type == ENTRY_DIR) {
        /* Navegar para subpasta */
        if (strcmp(s_current_dir, "/sdcard") == 0) {
            snprintf(s_current_dir, sizeof(s_current_dir), "/sdcard/%s", node->name);
        } else {
            snprintf(s_current_dir, sizeof(s_current_dir), "%s/%s", s_current_dir, node->name);
        }
        render_browser();
    } else {
        /* Abrir visualizador se for texto/config */
        show_file_viewer(node);
    }
}

static void up_btn_cb(lv_event_t *e)
{
    (void)e;
    if (strcmp(s_current_dir, "/sdcard") != 0) {
        /* Volta para a raiz */
        char *last_slash = strrchr(s_current_dir, '/');
        if (last_slash && last_slash != s_current_dir) {
            *last_slash = '\0';
        } else {
            strcpy(s_current_dir, "/sdcard");
        }
        render_browser();
    }
}

static void render_browser(void)
{
    if (s_viewer_view) {
        lv_obj_delete(s_viewer_view);
        s_viewer_view = NULL;
    }
    if (s_browser_view) {
        lv_obj_delete(s_browser_view);
        s_browser_view = NULL;
    }

    s_browser_view = lv_obj_create(s_parent_container);
    lv_obj_set_size(s_browser_view, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_opa(s_browser_view, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(s_browser_view, 0, 0);
    lv_obj_set_style_pad_all(s_browser_view, 0, 0);
    lv_obj_set_scrollable(s_browser_view, false);

    /* 1. Barra de Caminho e Navegação Superior */
    lv_obj_t *nav_bar = lv_obj_create(s_browser_view);
    lv_obj_set_size(nav_bar, LV_PCT(100), 44);
    lv_obj_align(nav_bar, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_bg_color(nav_bar, RUNESOS_COLOR_SURFACE, 0);
    lv_obj_set_style_radius(nav_bar, 10, 0);
    lv_obj_set_style_border_width(nav_bar, 1, 0);
    lv_obj_set_style_border_color(nav_bar, lv_color_hex(0x2A3B5C), 0);
    lv_obj_set_style_pad_hor(nav_bar, 10, 0);
    lv_obj_set_style_pad_ver(nav_bar, 4, 0);
    lv_obj_set_scrollable(nav_bar, false);
    lv_obj_set_flex_flow(nav_bar, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(nav_bar, LV_FLEX_ALIGN_SPACE_BETWEEN,
                          LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    s_path_label = lv_label_create(nav_bar);
    lv_label_set_text(s_path_label, s_current_dir);
    lv_obj_set_style_text_font(s_path_label, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(s_path_label, RUNESOS_COLOR_ACCENT, 0);

    /* Botão Subir */
    if (strcmp(s_current_dir, "/sdcard") != 0) {
        lv_obj_t *up_btn = lv_btn_create(nav_bar);
        lv_obj_set_size(up_btn, 80, 32);
        lv_obj_set_style_bg_color(up_btn, RUNESOS_COLOR_ACCENT, 0);
        lv_obj_set_style_radius(up_btn, 6, 0);
        lv_obj_t *ul = lv_label_create(up_btn);
        lv_label_set_text(ul, "⬆️ Subir");
        lv_obj_set_style_text_color(ul, lv_color_hex(0x1A1A2E), 0);
        lv_obj_set_style_text_font(ul, &lv_font_montserrat_12, 0);
        lv_obj_center(ul);
        lv_obj_add_event_cb(up_btn, up_btn_cb, LV_EVENT_CLICKED, NULL);
    }

    /* 2. Barra de Armazenamento */
    runesos_hal_sd_t sd;
    runesos_hal_mock.get_sd(&sd);

    lv_obj_t *storage_box = lv_obj_create(s_browser_view);
    lv_obj_set_size(storage_box, LV_PCT(100), 50);
    lv_obj_align(storage_box, LV_ALIGN_TOP_MID, 0, 48);
    lv_obj_set_style_bg_color(storage_box, RUNESOS_COLOR_SURFACE, 0);
    lv_obj_set_style_radius(storage_box, 8, 0);
    lv_obj_set_style_border_width(storage_box, 0, 0);
    lv_obj_set_style_pad_hor(storage_box, 12, 0);
    lv_obj_set_style_pad_ver(storage_box, 6, 0);
    lv_obj_set_scrollable(storage_box, false);

    lv_obj_t *st_lbl = lv_label_create(storage_box);
    char st_txt[64];
    char free_str[16], tot_str[16];
    runesos_format_size(sd.free_bytes, free_str, sizeof(free_str));
    runesos_format_size(sd.total_bytes, tot_str, sizeof(tot_str));
    snprintf(st_txt, sizeof(st_txt), "💾 SD Card: %s livres de %s", free_str, tot_str);
    lv_label_set_text(st_lbl, st_txt);
    lv_obj_set_style_text_font(st_lbl, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(st_lbl, RUNESOS_COLOR_TEXT, 0);
    lv_obj_align(st_lbl, LV_ALIGN_TOP_LEFT, 0, 0);

    lv_obj_t *bar = lv_bar_create(storage_box);
    lv_obj_set_size(bar, LV_PCT(100), 8);
    lv_obj_align(bar, LV_ALIGN_BOTTOM_MID, 0, 0);
    uint32_t used_pct = (uint32_t)(((sd.total_bytes - sd.free_bytes) * 100) / sd.total_bytes);
    lv_bar_set_value(bar, used_pct, LV_ANIM_OFF);
    lv_obj_set_style_bg_color(bar, RUNESOS_COLOR_ACCENT, LV_PART_INDICATOR);

    /* 3. Lista de Arquivos e Pastas */
    lv_obj_t *list = lv_obj_create(s_browser_view);
    lv_obj_set_size(list, LV_PCT(100), 430);
    lv_obj_align(list, LV_ALIGN_TOP_MID, 0, 104);
    lv_obj_set_style_bg_opa(list, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(list, 0, 0);
    lv_obj_set_style_pad_all(list, 2, 0);
    lv_obj_set_style_pad_row(list, 8, 0);
    lv_obj_set_flex_flow(list, LV_FLEX_FLOW_COLUMN);

    for (size_t i = 0; i < TOTAL_FS_NODES; i++) {
        if (strcmp(s_fs_nodes[i].parent_dir, s_current_dir) == 0) {
            lv_obj_t *item = lv_btn_create(list);
            lv_obj_set_size(item, LV_PCT(100), 54);
            lv_obj_set_style_bg_color(item, RUNESOS_COLOR_SURFACE, 0);
            lv_obj_set_style_radius(item, 10, 0);
            lv_obj_set_style_border_width(item, 1, 0);
            lv_obj_set_style_border_color(item, lv_color_hex(0x2A3B5C), 0);
            lv_obj_set_style_pad_hor(item, 12, 0);
            lv_obj_set_style_pad_ver(item, 6, 0);
            lv_obj_set_scrollable(item, false);

            lv_obj_set_flex_flow(item, LV_FLEX_FLOW_ROW);
            lv_obj_set_flex_align(item, LV_FLEX_ALIGN_SPACE_BETWEEN,
                                  LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

            /* Lado Esquerdo: Ícone + Nome */
            lv_obj_t *left_box = lv_obj_create(item);
            lv_obj_set_style_bg_opa(left_box, LV_OPA_TRANSP, 0);
            lv_obj_set_style_border_width(left_box, 0, 0);
            lv_obj_set_style_pad_all(left_box, 0, 0);
            lv_obj_set_flex_flow(left_box, LV_FLEX_FLOW_ROW);
            lv_obj_set_flex_align(left_box, LV_FLEX_ALIGN_START,
                                  LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
            lv_obj_set_style_pad_column(left_box, 10, 0);

            lv_obj_t *icon_lbl = lv_label_create(left_box);
            lv_label_set_text(icon_lbl, get_icon_for_type(s_fs_nodes[i].type));
            lv_obj_set_style_text_font(icon_lbl, &lv_font_montserrat_16, 0);

            lv_obj_t *name_lbl = lv_label_create(left_box);
            lv_label_set_text(name_lbl, s_fs_nodes[i].name);
            lv_obj_set_style_text_font(name_lbl, &lv_font_montserrat_14, 0);
            lv_obj_set_style_text_color(name_lbl, RUNESOS_COLOR_TEXT, 0);

            /* Lado Direito: Tamanho ou Indicador */
            lv_obj_t *right_lbl = lv_label_create(item);
            if (s_fs_nodes[i].type == ENTRY_DIR) {
                lv_label_set_text(right_lbl, "Pasta ❯");
                lv_obj_set_style_text_color(right_lbl, RUNESOS_COLOR_ACCENT, 0);
            } else {
                char sz_str[16];
                runesos_format_size(s_fs_nodes[i].size_bytes, sz_str, sizeof(sz_str));
                lv_label_set_text(right_lbl, sz_str);
                lv_obj_set_style_text_color(right_lbl, lv_color_hex(0x8888AA), 0);
            }
            lv_obj_set_style_text_font(right_lbl, &lv_font_montserrat_12, 0);

            lv_obj_add_event_cb(item, node_clicked_cb, LV_EVENT_CLICKED, (void *)&s_fs_nodes[i]);
        }
    }
}

static void show_file_viewer(const vfs_node_t *node)
{
    if (s_browser_view) {
        lv_obj_delete(s_browser_view);
        s_browser_view = NULL;
    }

    s_viewer_view = lv_obj_create(s_parent_container);
    lv_obj_set_size(s_viewer_view, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_opa(s_viewer_view, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(s_viewer_view, 0, 0);
    lv_obj_set_style_pad_all(s_viewer_view, 0, 0);
    lv_obj_set_scrollable(s_viewer_view, false);

    /* Cabeçalho do Leitor */
    lv_obj_t *hdr = lv_obj_create(s_viewer_view);
    lv_obj_set_size(hdr, LV_PCT(100), 40);
    lv_obj_align(hdr, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_bg_color(hdr, RUNESOS_COLOR_SURFACE, 0);
    lv_obj_set_style_border_width(hdr, 0, 0);
    lv_obj_set_style_pad_hor(hdr, 10, 0);
    lv_obj_set_flex_flow(hdr, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(hdr, LV_FLEX_ALIGN_SPACE_BETWEEN,
                          LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t *fn_lbl = lv_label_create(hdr);
    lv_label_set_text(fn_lbl, node->name);
    lv_obj_set_style_text_font(fn_lbl, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(fn_lbl, RUNESOS_COLOR_ACCENT, 0);

    lv_obj_t *close_btn = lv_btn_create(hdr);
    lv_obj_set_size(close_btn, 70, 30);
    lv_obj_set_style_bg_color(close_btn, RUNESOS_COLOR_ACCENT, 0);
    lv_obj_set_style_radius(close_btn, 6, 0);
    lv_obj_t *cl = lv_label_create(close_btn);
    lv_label_set_text(cl, "Fechar");
    lv_obj_set_style_text_color(cl, lv_color_hex(0x1A1A2E), 0);
    lv_obj_set_style_text_font(cl, &lv_font_montserrat_12, 0);
    lv_obj_center(cl);
    lv_obj_add_event_cb(close_btn, (lv_event_cb_t)render_browser, LV_EVENT_CLICKED, NULL);

    /* Área de Leitura de Texto */
    lv_obj_t *content_ta = lv_textarea_create(s_viewer_view);
    lv_obj_set_size(content_ta, LV_PCT(100), 480);
    lv_obj_align(content_ta, LV_ALIGN_TOP_MID, 0, 46);
    lv_obj_set_style_bg_color(content_ta, lv_color_hex(0x0E141E), 0);
    lv_obj_set_style_text_color(content_ta, RUNESOS_COLOR_TEXT, 0);
    lv_obj_set_style_text_font(content_ta, &lv_font_montserrat_12, 0);
    lv_obj_set_style_border_color(content_ta, lv_color_hex(0x2A3B5C), 0);

    if (node->content) {
        lv_textarea_set_text(content_ta, node->content);
    } else {
        lv_textarea_set_text(content_ta, "[Arquivo binario ou de midia - pre-visualizacao indisponivel]");
    }
}

void app_files_create(lv_obj_t *parent)
{
    s_parent_container = parent;
    strcpy(s_current_dir, "/sdcard");
    render_browser();
}

void app_files_destroy(void)
{
    s_parent_container = NULL;
    s_browser_view = NULL;
    s_viewer_view = NULL;
    s_path_label = NULL;
}
