/* fwMSX -- tokenizador minimo de XML (C), so' o que os arquivos de maquinas e extensoes do openMSX usam: tags com atributos, texto, comentarios.
   Codigo ORIGINAL do fwMSX (BSD-3-Clause). Nao valida nada: serve para ler dados que o proprio projeto conhece. Ver doc/romdb-spec.md. */
#ifndef FWMSX_XML_SCAN_H
#define FWMSX_XML_SCAN_H

#ifdef __cplusplus
extern "C" {
#endif

enum { XML_END = 0, XML_OPEN = 1, XML_CLOSE = 2, XML_TEXT = 3 };

typedef struct XmlTok {
    int kind;
    const char *name;  /* XML_OPEN/XML_CLOSE: o nome da tag */
    int name_len;
    const char *attrs; /* XML_OPEN: o texto depois do nome, ate o '>' (sem a barra final) */
    int attrs_len;
    const char *text;  /* XML_TEXT: o texto entre tags (nao vazio, sem espacos nas pontas) */
    int text_len;
} XmlTok;

/* O estado da leitura: onde esta' e o CLOSE que falta entregar depois de uma tag vazia. Zerar e preencher pos/end antes do primeiro xml_next. */
typedef struct XmlScan {
    const char *pos;
    const char *end;
    const char *pend_name;
    int pend_len;
} XmlScan;

/* Le o proximo token (s->pos avanca). Comentarios, <?...?> e <!DOCTYPE> sao pulados. Uma tag vazia (<code/>) devolve um OPEN
   e, na chamada seguinte, o CLOSE correspondente. Devolve o kind (XML_END no fim do texto). */
int xml_next(XmlScan *s, XmlTok *tok);

/* O valor do atributo `key` dentro de `attrs` (aspas simples ou duplas), copiado para `out` (terminado em zero, no maximo cap-1 bytes).
   Devolve o tamanho copiado, ou -1 se o atributo nao existe. */
int xml_attr(const char *attrs, int attrs_len, const char *key, char *out, int cap);

#ifdef __cplusplus
}
#endif
#endif
