/* md_parser.h */
#ifndef MD_PARSER_H
#define MD_PARSER_H

#include <stddef.h>

/* Converte Markdown (subset) em texto com tags do lv_richtext.
   Retorna o tamanho escrito, ou 0 se não couber no buffer. */
size_t md_to_richtext(const char *md, char *out, size_t out_size);

#endif