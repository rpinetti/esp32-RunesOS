/* md_parser.c */
#include "md_parser.h"

#include <string.h>

/* Tags geradas (o lv_richtext interpreta na hora de renderizar) */
#define TAG_H_OPEN    "<b><color #d4af37>"   /* títulos: bold + dourado */
#define TAG_H_CLOSE   "</color></b>"
#define TAG_B_OPEN    "<b>"
#define TAG_B_CLOSE   "</b>"
#define TAG_I_OPEN    "<i>"
#define TAG_I_CLOSE   "</i>"
#define TAG_C_OPEN    "<color #7fb3d5>"      /* código inline: azul claro */
#define TAG_C_CLOSE   "</color>"
#define TAG_U_OPEN    "<u>"                  /* links viram sublinhado */
#define TAG_U_CLOSE   "</u>"
#define TAG_BULLET    "<color #d4af37>•</color> "

typedef struct {
    char *out;          /* posição atual de escrita */
    const char *end;    /* último byte gravável (reserva o \0) */
    bool bold, italic, code;
} md_ctx_t;

static bool md_write(md_ctx_t *c, const char *s)
{
    size_t len = strlen(s);
    if (c->out + len > c->end)
        return false;
    memcpy(c->out, s, len);
    c->out += len;
    return true;
}

static void md_write_char(md_ctx_t *c, char ch)
{
    if (c->out >= c->end)
        return;
    /* `<` e `>` literais quebrariam o parser do richtext; trocamos por glifos seguros */
    if (ch == '<') ch = '‹';
    else if (ch == '>') ch = '›';
    *c->out++ = ch;
}

size_t md_to_richtext(const char *md, char *out, size_t out_size)
{
    if (!md || !out || out_size == 0)
        return 0;

    md_ctx_t c = { .out = out, .end = out + out_size - 1,
                   .bold = false, .italic = false, .code = false };
    const char *p = md;
    bool line_start = true;

    while (*p && c.out < c.end) {

        /* Título: # ## ### no início da linha */
        if (!c.code && line_start && *p == '#') {
            int n = 0;
            while (p[n] == '#' && n < 4) n++;
            if (n >= 1 && n <= 3 && p[n] == ' ') {
                if (!md_write(&c, TAG_H_OPEN)) break;
                for (p += n + 1; *p && *p != '\n' && c.out < c.end; p++)
                    md_write_char(&c, *p);
                if (!md_write(&c, TAG_H_CLOSE)) break;
                continue;
            }
        }

        /* Lista: "- ", "* " ou "1. " no início da linha */
        if (!c.code && line_start && p[1] == ' ' &&
            (p[0] == '-' || p[0] == '*')) {
            if (!md_write(&c, TAG_BULLET)) break;
            p += 2;
            line_start = false;
            continue;
        }
        if (!c.code && line_start && p[0] >= '0' && p[0] <= '9' &&
            p[1] == '.' && p[2] == ' ') {
            if (!md_write(&c, TAG_BULLET)) break;
            p += 3;
            line_start = false;
            continue;
        }

        /* Quebra de linha */
        if (*p == '\n') {
            if (!md_write(&c, "<br>")) break;
            p++;
            line_start = true;
            continue;
        }

        /* Escape "\x" → x literal */
        if (*p == '\\' && p[1]) {
            md_write_char(&c, p[1]);
            p += 2;
            line_start = false;
            continue;
        }

        /* Código inline: `texto` */
        if (*p == '`') {
            if (c.code) {
                if (!md_write(&c, TAG_C_CLOSE)) break;
                c.code = false;
            } else {
                if (!md_write(&c, TAG_C_OPEN)) break;
                c.code = true;
            }
            p++;
            continue;
        }

        /* Negrito: **texto** */
        if (!c.code && p[0] == '*' && p[1] == '*') {
            if (c.bold) {
                if (!md_write(&c, TAG_B_CLOSE)) break;
                c.bold = false;
            } else {
                if (!md_write(&c, TAG_B_OPEN)) break;
                c.bold = true;
            }
            p += 2;
            continue;
        }

        /* Itálico: *texto* */
        if (!c.code && *p == '*') {
            if (c.italic) {
                if (!md_write(&c, TAG_I_CLOSE)) break;
                c.italic = false;
            } else {
                if (!md_write(&c, TAG_I_OPEN)) break;
                c.italic = true;
            }
            p++;
            continue;
        }

        /* Link: [texto](url) → texto sublinhado (a URL é descartada) */
        if (!c.code && *p == '[') {
            const char *close = strchr(p, ']');
            const char *par   = close ? strchr(close, '(') : NULL;
            if (close && par && strchr(par, ')')) {
                if (!md_write(&c, TAG_U_OPEN)) break;
                for (p++; p < close && c.out < c.end; p++)
                    md_write_char(&c, *p);
                if (!md_write(&c, TAG_U_CLOSE)) break;
                p = strchr(par, ')') + 1;
                line_start = false;
                continue;
            }
        }

        /* Texto comum */
        md_write_char(&c, *p);
        if (*p != ' ') line_start = false;
        p++;
    }

    /* Fecha tags abertas para não quebrar a renderização */
    if (c.code)   md_write(&c, TAG_C_CLOSE);
    if (c.italic) md_write(&c, TAG_I_CLOSE);
    if (c.bold)   md_write(&c, TAG_B_CLOSE);

    *c.out = '\0';
    return (size_t)(c.out - out);
}