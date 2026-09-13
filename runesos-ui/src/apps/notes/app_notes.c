/* notes_app.c */
#include "app_notes.h"
#include "runesos_hal_storage.h"
#include "md_parser.h"

#include <string.h>

/* Paleta nórdica do RunesOS */
#define CLR_BG     0x14142a   /* azul-noite profundo */
#define CLR_CARD   0x1a1a2e   /* azul-noite */
#define CLR_CARD2  0x243b55   /* cartão destacado */
#define CLR_GOLD   0xd4af37   /* dourado */
#define CLR_TEXT   0xe8e8f0   /* texto claro */
#define CLR_SUB    0x8a8aaa   /* texto secundário */

typedef struct {
    lv_obj_t *base;
    lv_obj_t *list_screen;
    lv_obj_t *edit_screen;
    lv_obj_t *note_list;

    lv_obj_t *editor;        /* modo edição */
    lv_obj_t *preview;       /* modo preview (richtext) */
    lv_obj_t *lbl_name;      /* nome do arquivo no header */
    lv_obj_t *lbl_mode;      /* label do botão Editor/Preview */
    lv_obj_t *lbl_del;       /* label do botão Apagar (confirmação) */
    lv_obj_t *toast;         /* aviso temporário */

    lv_timer_t *toast_timer;
    lv_timer_t *confirm_timer;

    char  current[NOTES_MAX_NAME];  /* arquivo aberto ("x.md") */
    char *render;                   /* saída do parser (heap) */
    bool previewing;
    bool confirm_delete;
} notes_ctx_t;

/* ---- protótipos ---- */
static void notes_build_list(notes_ctx_t *ctx);
static void notes_build_editor(notes_ctx_t *ctx);
static lv_obj_t *notes_make_btn(lv_obj_t *parent, const char *txt,
                                lv_event_cb_t cb, void *ud, lv_obj_t **lbl_out);
static void notes_refresh_list(notes_ctx_t *ctx);
static void notes_enter_editor(notes_ctx_t *ctx);
static void notes_open(notes_ctx_t *ctx, const char *name);
static void notes_save(notes_ctx_t *ctx);
static void notes_new(notes_ctx_t *ctx);
static void notes_back(notes_ctx_t *ctx);
static void notes_toggle_mode(notes_ctx_t *ctx);
static void notes_gen_name(notes_ctx_t *ctx, const char *text);
static void notes_toast(notes_ctx_t *ctx, const char *msg);

/* ---- eventos ---- */
static void notes_on_delete(lv_event_t *e);
static void notes_on_open_note(lv_event_t *e);
static void notes_on_item_delete(lv_event_t *e);
static void notes_on_mode(lv_event_t *e);
static void notes_on_save(lv_event_t *e);
static void notes_on_delete_note(lv_event_t *e);
static void notes_on_back(lv_event_t *e);
static void notes_on_new(lv_event_t *e);

/* ================= criação / destruição ================= */

lv_obj_t *notes_app_create(lv_obj_t *parent)
{
    notes_ctx_t *ctx = lv_malloc(sizeof(notes_ctx_t));
    LV_ASSERT_MALLOC(ctx);
    if (!ctx) return NULL;
    memset(ctx, 0, sizeof(*ctx));

    ctx->render = lv_malloc(NOTES_MAX_LEN * 2); /* tags ocupam ~2x o texto */
    LV_ASSERT_MALLOC(ctx->render);
    if (!ctx->render) {
        lv_free(ctx);
        return NULL;
    }

    ctx->base = lv_obj_create(parent);
    lv_obj_set_size(ctx->base, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(ctx->base, lv_color_hex(CLR_BG), LV_PART_MAIN);
    lv_obj_set_style_border_width(ctx->base, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(ctx->base, 0, LV_PART_MAIN);
    lv_obj_remove_flag(ctx->base, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_event_cb(ctx->base, notes_on_delete, LV_EVENT_DELETE, ctx);

    notes_build_list(ctx);
    notes_build_editor(ctx);
    notes_refresh_list(ctx);

    ctx->confirm_timer = lv_timer_create(notes_confirm_timer_cb, 3000, ctx);
    lv_timer_pause(ctx->confirm_timer);   /* ativa só quando pedir para apagar */
    ctx->toast_timer = lv_timer_create(notes_toast_timer_cb, 1500, ctx);
    lv_timer_pause(ctx->toast_timer);

    return ctx->base;
}

void notes_app_destroy(lv_obj_t *obj)
{
    lv_obj_delete(obj); /* o ctx é liberado em notes_on_delete */
}

static void notes_on_delete(lv_event_t *e)
{
    notes_ctx_t *ctx = lv_event_get_user_data(e);
    if (!ctx) return;
    if (ctx->confirm_timer) lv_timer_delete(ctx->confirm_timer);
    if (ctx->toast_timer)   lv_timer_delete(ctx->toast_timer);
    lv_free(ctx->render);
    lv_free(ctx);
}

/* ================= lista de notas ================= */

static void notes_build_list(notes_ctx_t *ctx)
{
    lv_obj_t *scr = lv_obj_create(ctx->base);
    lv_obj_set_size(scr, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(scr, lv_color_hex(CLR_BG), LV_PART_MAIN);
    lv_obj_set_style_border_width(scr, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(scr, 0, LV_PART_MAIN);
    lv_obj_remove_flag(scr, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_flex_flow(scr, LV_FLEX_FLOW_COLUMN);
    ctx->list_screen = scr;

    /* header */
    lv_obj_t *hdr = lv_obj_create(scr);
    lv_obj_set_size(hdr, LV_PCT(100), 48);
    lv_obj_set_style_bg_color(hdr, lv_color_hex(CLR_CARD), LV_PART_MAIN);
    lv_obj_set_style_border_width(hdr, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(hdr, 0, LV_PART_MAIN);

    lv_obj_t *t = lv_label_create(hdr);
    lv_label_set_text(t, "✦ Notas");
    lv_obj_set_style_text_color(t, lv_color_hex(CLR_GOLD), LV_PART_MAIN);
    lv_obj_center(t);

    /* corpo: lista rolável */
    lv_obj_t *list = lv_obj_create(scr);
    lv_obj_set_flex_grow(list, 1);
    lv_obj_set_width(list, LV_PCT(100));
    lv_obj_set_flex_flow(list, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(list, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER,
                          LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_bg_color(list, lv_color_hex(CLR_BG), LV_PART_MAIN);
    lv_obj_set_style_border_width(list, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(list, 6, LV_PART_MAIN);
    lv_obj_set_style_pad_row(list, 6, LV_PART_MAIN);
    ctx->note_list = list;

    /* barra inferior: nova nota */
    lv_obj_t *bar = lv_obj_create(scr);
    lv_obj_set_size(bar, LV_PCT(100), 56);
    lv_obj_set_style_bg_color(bar, lv_color_hex(CLR_CARD), LV_PART_MAIN);
    lv_obj_set_style_border_width(bar, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(bar, 6, LV_PART_MAIN);

    lv_obj_t *b = notes_make_btn(bar, "＋ Nova nota", notes_on_new, ctx, NULL);
    lv_obj_set_size(b, LV_PCT(100), LV_PCT(100));
}

static void notes_refresh_list(notes_ctx_t *ctx)
{
    while (lv_obj_get_child_count(ctx->note_list) > 0)
        lv_obj_delete(lv_obj_get_child(ctx->note_list, 0));

    char names[NOTES_MAX_NOTES][NOTES_MAX_NAME];
    int n = runesos_hal_storage_get()->dir_list(NOTES_DIR, names, NOTES_MAX_NOTES);

    for (int i = 0; i < n; i++) {
        lv_obj_t *btn = lv_button_create(ctx->note_list);
        lv_obj_set_width(btn, LV_PCT(100));
        lv_obj_set_height(btn, 44);
        lv_obj_set_style_bg_color(btn, lv_color_hex(CLR_CARD), LV_PART_MAIN);
        lv_obj_set_style_bg_color(btn, lv_color_hex(CLR_CARD2), LV_STATE_PRESSED);
        lv_obj_set_style_border_color(btn, lv_color_hex(0x2a2a4a), LV_PART_MAIN);
        lv_obj_set_style_border_width(btn, 1, LV_PART_MAIN);
        lv_obj_set_style_radius(btn, 8, LV_PART_MAIN);

        lv_obj_t *lbl = lv_label_create(btn);
        lv_label_set_text(lbl, names[i]);
        lv_obj_set_style_text_color(lbl, lv_color_hex(CLR_TEXT), LV_PART_MAIN);
        lv_obj_set_style_text_align(lbl, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN);
        lv_obj_center(lbl);

        /* o nome fica no user_data do botão; liberado no delete */
        lv_obj_set_user_data(btn, lv_strdup(names[i]));
        lv_obj_add_event_cb(btn, notes_on_open_note, LV_EVENT_CLICKED, ctx);
        lv_obj_add_event_cb(btn, notes_on_item_delete, LV_EVENT_DELETE, NULL);
    }
}

static void notes_on_item_delete(lv_event_t *e)
{
    lv_obj_t *obj = lv_event_get_target(e);
    void *ud = lv_obj_get_user_data(obj);
    if (ud) {
        lv_free(ud);
        lv_obj_set_user_data(obj, NULL);
    }
}

static void notes_on_open_note(lv_event_t *e)
{
    notes_ctx_t *ctx = lv_event_get_user_data(e);
    const char *name = lv_obj_get_user_data(lv_event_get_target(e));
    if (name) notes_open(ctx, name);
}

/* ================= editor ================= */

static void notes_build_editor(notes_ctx_t *ctx)
{
    lv_obj_t *scr = lv_obj_create(ctx->base);
    lv_obj_set_size(scr, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(scr, lv_color_hex(CLR_BG), LV_PART_MAIN);
    lv_obj_set_style_border_width(scr, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(scr, 0, LV_PART_MAIN);
    lv_obj_remove_flag(scr, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_flex_flow(scr, LV_FLEX_FLOW_COLUMN);
    lv_obj_add_flag(scr, LV_OBJ_FLAG_HIDDEN);
    ctx->edit_screen = scr;

    /* header com o nome do arquivo */
    lv_obj_t *hdr = lv_obj_create(scr);
    lv_obj_set_size(hdr, LV_PCT(100), 48);
    lv_obj_set_style_bg_color(hdr, lv_color_hex(CLR_CARD), LV_PART_MAIN);
    lv_obj_set_style_border_width(hdr, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(hdr, 0, LV_PART_MAIN);

    ctx->lbl_name = lv_label_create(hdr);
    lv_label_set_text(ctx->lbl_name, "—");
    lv_obj_set_style_text_color(ctx->lbl_name, lv_color_hex(CLR_GOLD), LV_PART_MAIN);
    lv_obj_center(ctx->lbl_name);

    /* corpo: textarea + richtext empilhados (só um visível por vez) */
    lv_obj_t *body = lv_obj_create(scr);
    lv_obj_set_flex_grow(body, 1);
    lv_obj_set_width(body, LV_PCT(100));
    lv_obj_set_style_bg_color(body, lv_color_hex(CLR_BG), LV_PART_MAIN);
    lv_obj_set_style_border_width(body, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(body, 8, LV_PART_MAIN);

    ctx->editor = lv_textarea_create(body);
    lv_obj_set_size(ctx->editor, LV_PCT(100), LV_PCT(100));
    lv_textarea_set_placeholder_text(ctx->editor, "Escreva em Markdown…");
    lv_textarea_set_cursor_click_pos(ctx->editor, true);
    lv_textarea_set_text_selection(ctx->editor, true);
    lv_obj_set_style_bg_color(ctx->editor, lv_color_hex(CLR_CARD), LV_PART_MAIN);
    lv_obj_set_style_border_color(ctx->editor, lv_color_hex(CLR_GOLD), LV_PART_MAIN);
    lv_obj_set_style_border_width(ctx->editor, 1, LV_PART_MAIN);
    lv_obj_set_style_radius(ctx->editor, 8, LV_PART_MAIN);
    lv_obj_set_style_text_color(ctx->editor, lv_color_hex(CLR_TEXT), LV_PART_MAIN);
    lv_obj_set_style_pad_all(ctx->editor, 8, LV_PART_MAIN);

    ctx->preview = lv_richtext_create(body);
    lv_obj_set_size(ctx->preview, LV_PCT(100), LV_PCT(100));
    lv_obj_add_flag(ctx->preview, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_style_bg_color(ctx->preview, lv_color_hex(CLR_CARD), LV_PART_MAIN);
    lv_obj_set_style_border_color(ctx->preview, lv_color_hex(0x2a2a4a), LV_PART_MAIN);
    lv_obj_set_style_border_width(ctx->preview, 1, LV_PART_MAIN);
    lv_obj_set_style_radius(ctx->preview, 8, LV_PART_MAIN);
    lv_obj_set_style_pad_all(ctx->preview, 8, LV_PART_MAIN);
    lv_obj_set_style_text_color(ctx->preview, lv_color_hex(CLR_TEXT), LV_PART_MAIN);

    /* barra de ações: Preview | Salvar | Apagar | Voltar */
    lv_obj_t *bar = lv_obj_create(scr);
    lv_obj_set_size(bar, LV_PCT(100), 60);
    lv_obj_set_style_bg_color(bar, lv_color_hex(CLR_CARD), LV_PART_MAIN);
    lv_obj_set_style_border_width(bar, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(bar, 6, LV_PART_MAIN);
    lv_obj_set_flex_flow(bar, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(bar, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER,
                          LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(bar, 6, LV_PART_MAIN);

    lv_obj_t *b1 = notes_make_btn(bar, "Preview", notes_on_mode, ctx, &ctx->lbl_mode);
    lv_obj_t *b2 = notes_make_btn(bar, "Salvar",  notes_on_save, ctx, NULL);
    lv_obj_t *b3 = notes_make_btn(bar, "Apagar",  notes_on_delete_note, ctx, &ctx->lbl_del);
    lv_obj_t *b4 = notes_make_btn(bar, "← Voltar", notes_on_back, ctx, NULL);
    (void)b1; (void)b2; (void)b3; (void)b4;
}

static lv_obj_t *notes_make_btn(lv_obj_t *parent, const char *txt,
                                lv_event_cb_t cb, void *ud, lv_obj_t **lbl_out)
{
    lv_obj_t *btn = lv_button_create(parent);
    lv_obj_set_height(btn, LV_PCT(100));
    lv_obj_set_style_bg_color(btn, lv_color_hex(CLR_CARD2), LV_PART_MAIN);
    lv_obj_set_style_bg_color(btn, lv_color_hex(CLR_GOLD), LV_STATE_PRESSED);
    lv_obj_set_style_radius(btn, 8, LV_PART_MAIN);
    lv_obj_set_style_border_width(btn, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_left(btn, 12, LV_PART_MAIN);
    lv_obj_set_style_pad_right(btn, 12, LV_PART_MAIN);

    lv_obj_t *lbl = lv_label_create(btn);
    lv_label_set_text(lbl, txt);
    lv_obj_set_style_text_color(lbl, lv_color_hex(CLR_TEXT), LV_PART_MAIN);
    lv_obj_center(lbl);

    lv_obj_add_event_cb(btn, cb, LV_EVENT_CLICKED, ud);
    if (lbl_out) *lbl_out = lbl;
    return btn;
}

/* ================= navegação e ações ================= */

static void notes_enter_editor(notes_ctx_t *ctx)
{
    lv_obj_remove_flag(ctx->edit_screen, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(ctx->list_screen, LV_OBJ_FLAG_HIDDEN);
    lv_obj_move_foreground(ctx->edit_screen);
}

static void notes_open(notes_ctx_t *ctx, const char *name)
{
    lv_snprintf(ctx->current, sizeof(ctx->current), "%s", name);

    char *buf = lv_malloc(NOTES_MAX_LEN);
    if (!buf) return;
    buf[0] = '\0';

    const runesos_hal_storage_t *hal = runesos_hal_storage_get();
    char path[96];
    lv_snprintf(path, sizeof(path), NOTES_DIR "/%s", name);
    if (hal && hal->file_exists(path)) {
        size_t len = 0;
        hal->file_read(path, buf, NOTES_MAX_LEN, &len);
    }
    buf[NOTES_MAX_LEN - 1] = '\0';

    lv_textarea_set_text(ctx->editor, buf);
    lv_free(buf);
    lv_label_set_text(ctx->lbl_name, name);

    /* abre sempre no modo edição */
    lv_obj_remove_flag(ctx->editor, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(ctx->preview, LV_OBJ_FLAG_HIDDEN);
    ctx->previewing = false;
    lv_label_set_text(ctx->lbl_mode, "Preview");

    notes_enter_editor(ctx);
    lv_textarea_set_cursor_pos(ctx->editor, LV_TEXTAREA_CURSOR_LAST);
}

static void notes_new(notes_ctx_t *ctx)
{
    ctx->current[0] = '\0';
    ctx->confirm_delete = false;
    lv_label_set_text(ctx->lbl_del, "Apagar");

    lv_textarea_set_text(ctx->editor, "");
    lv_label_set_text(ctx->lbl_name, "Nova nota");
    lv_obj_remove_flag(ctx->editor, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(ctx->preview, LV_OBJ_FLAG_HIDDEN);
    ctx->previewing = false;
    lv_label_set_text(ctx->lbl_mode, "Preview");

    notes_enter_editor(ctx);
}

static void notes_back(notes_ctx_t *ctx)
{
    ctx->current[0] = '\0';
    ctx->confirm_delete = false;
    lv_label_set_text(ctx->lbl_del, "Apagar");

    lv_obj_add_flag(ctx->edit_screen, LV_OBJ_FLAG_HIDDEN);
    lv_obj_remove_flag(ctx->list_screen, LV_OBJ_FLAG_HIDDEN);
    notes_refresh_list(ctx);   /* recarrega a lista (surgiram notas novas?) */
}

static void notes_gen_name(notes_ctx_t *ctx, const char *text)
{
    /* a primeira linha vira o nome do arquivo */
    char name[NOTES_MAX_NAME];
    int i = 0;
    while (text[i] && text[i] != '\n' && i < (int)sizeof(name) - 6) {
        char ch = text[i];
        if (ch == '/' || ch == '\\' || ch == ':' || ch == '*' || ch == '?' ||
            ch == '"' || ch == '<' || ch == '>' || ch == '|' || ch == '#')
            ch = '_';
        if ((unsigned char)ch < 0x20) ch = '_';
        name[i++] = ch;
    }
    while (i > 0 && (name[i - 1] == ' ' || name[i - 1] == '.')) i--;
    if (i == 0) lv_snprintf(name, sizeof(name), "nota");
    name[i] = '\0';

    /* garante unicidade: nota.md, nota_1.md, nota_2.md… */
    int n = 1;
    lv_snprintf(ctx->current, sizeof(ctx->current), "%s.md", name);
    char path[96];
    for (;;) {
        lv_snprintf(path, sizeof(path), NOTES_DIR "/%s", ctx->current);
        if (!runesos_hal_storage_get()->file_exists(path)) break;
        lv_snprintf(ctx->current, sizeof(ctx->current), "%s_%d.md", name, n++);
    }
}

static void notes_save(notes_ctx_t *ctx)
{
    const char *text = lv_textarea_get_text(ctx->editor);
    size_t len = strlen(text);
    if (len == 0) {
        notes_toast(ctx, "Nota vazia — nada a salvar");
        return;
    }
    if (len >= NOTES_MAX_LEN) {
        notes_toast(ctx, "Nota acima do limite");
        return;
    }
    if (ctx->current[0] == '\0')   /* não é um arquivo aberto? vira uma nota nova */
        notes_gen_name(ctx, text);

    char path[96];
    lv_snprintf(path, sizeof(path), NOTES_DIR "/%s", ctx->current);
    if (runesos_hal_storage_get()->file_write(path, text, len))
        notes_toast(ctx, ctx->current);
    else
        notes_toast(ctx, "Falha ao salvar");
}

static void notes_toggle_mode(notes_ctx_t *ctx)
{
    ctx->previewing = !ctx->previewing;
    if (ctx->previewing) {
        const char *text = lv_textarea_get_text(ctx->editor);
        size_t n = md_to_richtext(text, ctx->render, NOTES_MAX_LEN * 2);
        if (n == 0) notes_toast(ctx, "Nota longa demais");
        lv_richtext_set_text(ctx->preview, ctx->render);
        lv_obj_remove_flag(ctx->preview, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(ctx->editor, LV_OBJ_FLAG_HIDDEN);
        lv_label_set_text(ctx->lbl_mode, "Editar");
    } else {
        lv_obj_remove_flag(ctx->editor, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(ctx->preview, LV_OBJ_FLAG_HIDDEN);
        lv_label_set_text(ctx->lbl_mode, "Preview");
    }
}

/* ================= eventos de ação ================= */

static void notes_on_mode(lv_event_t *e)
{
    notes_toggle_mode(lv_event_get_user_data(e));
}

static void notes_on_save(lv_event_t *e)
{
    notes_save(lv_event_get_user_data(e));
}

static void notes_on_back(lv_event_t *e)
{
    notes_back(lv_event_get_user_data(e));
}

static void notes_on_new(lv_event_t *e)
{
    notes_new(lv_event_get_user_data(e));
}

static void notes_on_delete_note(lv_event_t *e)
{
    notes_ctx_t *ctx = lv_event_get_user_data(e);

    /* 1º toque: pede confirmação por 3 s. 2º toque: apaga */
    if (!ctx->confirm_delete) {
        ctx->confirm_delete = true;
        lv_label_set_text(ctx->lbl_del, "Apagar?");
        lv_timer_reset(ctx->confirm_timer);
        lv_timer_resume(ctx->confirm_timer);
        lv_timer_set_repeat_count(ctx->confirm_timer, 1);
        return;
    }

    lv_timer_pause(ctx->confirm_timer);
    ctx->confirm_delete = false;
    lv_label_set_text(ctx->lbl_del, "Apagar");

    char path[96];
    lv_snprintf(path, sizeof(path), NOTES_DIR "/%s", ctx->current);
    const char *name = ctx->current;
    notes_back(ctx);
    if (runesos_hal_storage_get()->file_delete(path))
        notes_toast(ctx, "Apagado: ");
    else
        notes_toast(ctx, "Falha ao apagar");
    (void)name;
}

/* ================= timers ================= */

static void notes_confirm_timer_cb(lv_timer_t *t)
{
    notes_ctx_t *ctx = lv_timer_get_user_data(t);
    ctx->confirm_delete = false;
    lv_label_set_text(ctx->lbl_del, "Apagar");
    lv_timer_pause(t);
}

static void notes_toast_timer_cb(lv_timer_t *t)
{
    notes_ctx_t *ctx = lv_timer_get_user_data(t);
    if (ctx->toast) lv_obj_add_flag(ctx->toast, LV_OBJ_FLAG_HIDDEN);
    lv_timer_pause(t);
}

static void notes_toast(notes_ctx_t *ctx, const char *msg)
{
    if (!ctx->toast) {
        ctx->toast = lv_label_create(ctx->edit_screen);
        lv_obj_set_style_bg_color(ctx->toast, lv_color_hex(CLR_GOLD), LV_PART_MAIN);
        lv_obj_set_style_text_color(ctx->toast, lv_color_hex(0x14142a), LV_PART_MAIN);
        lv_obj_set_style_radius(ctx->toast, 8, LV_PART_MAIN);
        lv_obj_set_style_pad_all(ctx->toast, 8, LV_PART_MAIN);
        lv_obj_align(ctx->toast, LV_ALIGN_TOP_MID, 0, 56);
        lv_obj_add_flag(ctx->toast, LV_OBJ_FLAG_HIDDEN);
    }
    lv_label_set_text(ctx->toast, msg);
    lv_obj_remove_flag(ctx->toast, LV_OBJ_FLAG_HIDDEN);
    lv_obj_move_foreground(ctx->toast);
    lv_timer_reset(ctx->toast_timer);
    lv_timer_resume(ctx->toast_timer);
}

/* ================= registro no launcher ================= */

const runesos_app_t notes_app = {
    .name    = "Notas",
    .icon    = NULL,   /* launcher usa o ícone padrão até você gerar um */
    .create  = notes_app_create,
    .destroy = notes_app_destroy,
};