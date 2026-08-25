#include "runesos_app.h"
#include "theme_nordico.h"
#include <stdio.h>
#include <string.h>

#define MAX_NOTES 8
#define NOTE_TITLE_LEN 32
#define NOTE_BODY_LEN  512

typedef struct {
    char title[NOTE_TITLE_LEN];
    char body[NOTE_BODY_LEN];
    bool active;
} note_item_t;

static note_item_t s_notes[MAX_NOTES] = {
    {
        .title = "Bem-vindo ao RunesOS",
        .body = "Este e o seu cyberdeck portatil em uma lata de balas.\n"
                "Sinta-se livre para criar, editar e organizar suas ideias e runas aqui.",
        .active = true
    },
    {
        .title = "Segredos do Elder Futhark",
        .body = "1. Fehu (Riqueza)\n2. Uruz (Forca)\n3. Thurisaz (Protecao)\n4. Ansuz (Sabedoria)",
        .active = true
    },
    {
        .title = "Lista de Tarefas Cyberdeck",
        .body = "- [x] Unificar Statusbar\n- [x] Implementar Bloco de Notas\n- [ ] Conectar WiFi real\n- [ ] Soldar bateria LiPo",
        .active = true
    }
};

static lv_obj_t *s_parent_container = NULL;
static lv_obj_t *s_list_view = NULL;
static lv_obj_t *s_editor_view = NULL;
static lv_obj_t *s_title_ta = NULL;
static lv_obj_t *s_body_ta = NULL;
static lv_obj_t *s_keyboard = NULL;
static int       s_editing_index = -1;

static void show_list_view(void);
static void show_editor_view(int note_idx);

static void note_item_clicked_cb(lv_event_t *e)
{
    uintptr_t idx = (uintptr_t)lv_event_get_user_data(e);
    show_editor_view((int)idx);
}

static void new_note_btn_cb(lv_event_t *e)
{
    (void)e;
    /* Procura slot livre */
    int free_slot = -1;
    for (int i = 0; i < MAX_NOTES; i++) {
        if (!s_notes[i].active) {
            free_slot = i;
            break;
        }
    }
    if (free_slot != -1) {
        snprintf(s_notes[free_slot].title, NOTE_TITLE_LEN, "Nova Nota %d", free_slot + 1);
        s_notes[free_slot].body[0] = '\0';
        s_notes[free_slot].active = true;
        show_editor_view(free_slot);
    }
}

static void save_note_btn_cb(lv_event_t *e)
{
    (void)e;
    if (s_editing_index >= 0 && s_editing_index < MAX_NOTES) {
        if (s_title_ta) {
            const char *t = lv_textarea_get_text(s_title_ta);
            strncpy(s_notes[s_editing_index].title, t, NOTE_TITLE_LEN - 1);
            s_notes[s_editing_index].title[NOTE_TITLE_LEN - 1] = '\0';
        }
        if (s_body_ta) {
            const char *b = lv_textarea_get_text(s_body_ta);
            strncpy(s_notes[s_editing_index].body, b, NOTE_BODY_LEN - 1);
            s_notes[s_editing_index].body[NOTE_BODY_LEN - 1] = '\0';
        }
    }
    show_list_view();
}

static void delete_note_btn_cb(lv_event_t *e)
{
    (void)e;
    if (s_editing_index >= 0 && s_editing_index < MAX_NOTES) {
        s_notes[s_editing_index].active = false;
    }
    show_list_view();
}

static void ta_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *ta = lv_event_get_target(e);
    if (code == LV_EVENT_FOCUSED) {
        if (s_keyboard) {
            lv_keyboard_set_textarea(s_keyboard, ta);
            lv_obj_set_hidden(s_keyboard, false);
        }
    } else if (code == LV_EVENT_DEFOCUSED) {
        /* Keep or hide keyboard */
    }
}

static void show_list_view(void)
{
    if (s_editor_view) {
        lv_obj_delete(s_editor_view);
        s_editor_view = NULL;
    }
    if (s_list_view) {
        lv_obj_delete(s_list_view);
        s_list_view = NULL;
    }

    s_list_view = lv_obj_create(s_parent_container);
    lv_obj_set_size(s_list_view, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_opa(s_list_view, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(s_list_view, 0, 0);
    lv_obj_set_style_pad_all(s_list_view, 0, 0);
    lv_obj_set_scrollable(s_list_view, false);

    /* Botão Nova Nota */
    lv_obj_t *new_btn = lv_btn_create(s_list_view);
    lv_obj_set_size(new_btn, LV_PCT(100), 44);
    lv_obj_align(new_btn, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_bg_color(new_btn, RUNESOS_COLOR_ACCENT, 0);
    lv_obj_set_style_radius(new_btn, 10, 0);

    lv_obj_t *new_lbl = lv_label_create(new_btn);
    lv_label_set_text(new_lbl, "+ Nova Nota");
    lv_obj_set_style_text_font(new_lbl, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(new_lbl, lv_color_hex(0x1A1A2E), 0);
    lv_obj_center(new_lbl);

    lv_obj_add_event_cb(new_btn, new_note_btn_cb, LV_EVENT_CLICKED, NULL);

    /* Lista com scroll */
    lv_obj_t *scroll_list = lv_obj_create(s_list_view);
    lv_obj_set_size(scroll_list, LV_PCT(100), 480);
    lv_obj_align(scroll_list, LV_ALIGN_TOP_MID, 0, 52);
    lv_obj_set_style_bg_opa(scroll_list, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(scroll_list, 0, 0);
    lv_obj_set_style_pad_all(scroll_list, 4, 0);
    lv_obj_set_style_pad_row(scroll_list, 10, 0);
    lv_obj_set_flex_flow(scroll_list, LV_FLEX_FLOW_COLUMN);

    for (int i = 0; i < MAX_NOTES; i++) {
        if (s_notes[i].active) {
            lv_obj_t *card = lv_btn_create(scroll_list);
            lv_obj_set_size(card, LV_PCT(100), 68);
            lv_obj_set_style_bg_color(card, RUNESOS_COLOR_SURFACE, 0);
            lv_obj_set_style_radius(card, 12, 0);
            lv_obj_set_style_border_width(card, 1, 0);
            lv_obj_set_style_border_color(card, lv_color_hex(0x2A3B5C), 0);
            lv_obj_set_style_pad_hor(card, 14, 0);
            lv_obj_set_style_pad_ver(card, 8, 0);

            lv_obj_set_flex_flow(card, LV_FLEX_FLOW_COLUMN);
            lv_obj_set_flex_align(card, LV_FLEX_ALIGN_CENTER,
                                  LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);

            /* Título da Nota */
            lv_obj_t *title = lv_label_create(card);
            lv_label_set_text(title, s_notes[i].title);
            lv_obj_set_style_text_font(title, &lv_font_montserrat_16, 0);
            lv_obj_set_style_text_color(title, RUNESOS_COLOR_ACCENT, 0);

            /* Resumo / Preview do Corpo */
            lv_obj_t *preview = lv_label_create(card);
            char prev_buf[48];
            snprintf(prev_buf, sizeof(prev_buf), "%.40s...", s_notes[i].body);
            lv_label_set_text(preview, prev_buf);
            lv_obj_set_style_text_font(preview, &lv_font_montserrat_12, 0);
            lv_obj_set_style_text_color(preview, lv_color_hex(0x8888AA), 0);

            lv_obj_add_event_cb(card, note_item_clicked_cb, LV_EVENT_CLICKED, (void *)(uintptr_t)i);
        }
    }
}

static void show_editor_view(int note_idx)
{
    s_editing_index = note_idx;

    if (s_list_view) {
        lv_obj_delete(s_list_view);
        s_list_view = NULL;
    }
    if (s_editor_view) {
        lv_obj_delete(s_editor_view);
        s_editor_view = NULL;
    }

    s_editor_view = lv_obj_create(s_parent_container);
    lv_obj_set_size(s_editor_view, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_opa(s_editor_view, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(s_editor_view, 0, 0);
    lv_obj_set_style_pad_all(s_editor_view, 0, 0);
    lv_obj_set_scrollable(s_editor_view, false);

    /* Barra de Ações do Editor */
    lv_obj_t *action_bar = lv_obj_create(s_editor_view);
    lv_obj_set_size(action_bar, LV_PCT(100), 40);
    lv_obj_align(action_bar, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_bg_opa(action_bar, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(action_bar, 0, 0);
    lv_obj_set_style_pad_all(action_bar, 0, 0);
    lv_obj_set_flex_flow(action_bar, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(action_bar, LV_FLEX_ALIGN_SPACE_BETWEEN,
                          LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    /* Botão Voltar/Cancelar */
    lv_obj_t *cancel_btn = lv_btn_create(action_bar);
    lv_obj_set_size(cancel_btn, 80, 36);
    lv_obj_set_style_bg_color(cancel_btn, lv_color_hex(0x334466), 0);
    lv_obj_set_style_radius(cancel_btn, 8, 0);
    lv_obj_t *c_lbl = lv_label_create(cancel_btn);
    lv_label_set_text(c_lbl, "Lista");
    lv_obj_center(c_lbl);
    lv_obj_add_event_cb(cancel_btn, (lv_event_cb_t)show_list_view, LV_EVENT_CLICKED, NULL);

    /* Botão Excluir */
    lv_obj_t *del_btn = lv_btn_create(action_bar);
    lv_obj_set_size(del_btn, 80, 36);
    lv_obj_set_style_bg_color(del_btn, lv_color_hex(0x8B2635), 0);
    lv_obj_set_style_radius(del_btn, 8, 0);
    lv_obj_t *d_lbl = lv_label_create(del_btn);
    lv_label_set_text(d_lbl, "Excluir");
    lv_obj_center(d_lbl);
    lv_obj_add_event_cb(del_btn, delete_note_btn_cb, LV_EVENT_CLICKED, NULL);

    /* Botão Salvar */
    lv_obj_t *save_btn = lv_btn_create(action_bar);
    lv_obj_set_size(save_btn, 80, 36);
    lv_obj_set_style_bg_color(save_btn, RUNESOS_COLOR_ACCENT, 0);
    lv_obj_set_style_radius(save_btn, 8, 0);
    lv_obj_t *s_lbl = lv_label_create(save_btn);
    lv_label_set_text(s_lbl, "Salvar");
    lv_obj_set_style_text_color(s_lbl, lv_color_hex(0x1A1A2E), 0);
    lv_obj_center(s_lbl);
    lv_obj_add_event_cb(save_btn, save_note_btn_cb, LV_EVENT_CLICKED, NULL);

    /* Campo de Título */
    s_title_ta = lv_textarea_create(s_editor_view);
    lv_obj_set_size(s_title_ta, LV_PCT(100), 40);
    lv_obj_align(s_title_ta, LV_ALIGN_TOP_MID, 0, 46);
    lv_textarea_set_one_line(s_title_ta, true);
    lv_textarea_set_text(s_title_ta, s_notes[note_idx].title);
    lv_obj_set_style_bg_color(s_title_ta, RUNESOS_COLOR_SURFACE, 0);
    lv_obj_set_style_text_color(s_title_ta, RUNESOS_COLOR_TEXT, 0);
    lv_obj_set_style_border_color(s_title_ta, RUNESOS_COLOR_ACCENT, LV_STATE_FOCUSED);
    lv_obj_add_event_cb(s_title_ta, ta_event_cb, LV_EVENT_ALL, NULL);

    /* Campo de Conteúdo */
    s_body_ta = lv_textarea_create(s_editor_view);
    lv_obj_set_size(s_body_ta, LV_PCT(100), 220);
    lv_obj_align(s_body_ta, LV_ALIGN_TOP_MID, 0, 92);
    lv_textarea_set_text(s_body_ta, s_notes[note_idx].body);
    lv_obj_set_style_bg_color(s_body_ta, RUNESOS_COLOR_SURFACE, 0);
    lv_obj_set_style_text_color(s_body_ta, RUNESOS_COLOR_TEXT, 0);
    lv_obj_set_style_border_color(s_body_ta, RUNESOS_COLOR_ACCENT, LV_STATE_FOCUSED);
    lv_obj_add_event_cb(s_body_ta, ta_event_cb, LV_EVENT_ALL, NULL);

    /* Teclado Virtual Integrado */
    s_keyboard = lv_keyboard_create(s_editor_view);
    lv_obj_set_size(s_keyboard, LV_PCT(100), 200);
    lv_obj_align(s_keyboard, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_keyboard_set_textarea(s_keyboard, s_body_ta);
    lv_obj_set_style_bg_color(s_keyboard, RUNESOS_COLOR_SURFACE, 0);
}

void app_notas_create(lv_obj_t *parent)
{
    s_parent_container = parent;
    show_list_view();
}

void app_notas_destroy(void)
{
    s_parent_container = NULL;
    s_list_view = NULL;
    s_editor_view = NULL;
    s_title_ta = NULL;
    s_body_ta = NULL;
    s_keyboard = NULL;
    s_editing_index = -1;
}

