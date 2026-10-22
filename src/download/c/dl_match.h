// fwMSX -- downloads: comparacao de texto sem diferenciar maiusculas (C). Ver dl_match.c.
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// 1 se hay[0, hlen) contem needle[0, nlen) (ASCII A-Z dobrado para a-z; os demais bytes comparam iguais); needle vazio = 1.
int dl_contains_ci(const char *hay, int hlen, const char *needle, int nlen);

#ifdef __cplusplus
}
#endif
