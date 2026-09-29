//
// Modulo Fortran do msxdisk (fwMSX): calculos de geometria/capacidade.
//
#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Calcula estatisticas de capacidade de um disco FAT12 a partir do total
// de clusters de DADOS (exclui reservados/FAT/diretorio) e de quantos
// estao em uso, devolvendo espaco livre em bytes e percentual livre
// (0.0f a 100.0f).
void msxdisk_geometry_stats(int32_t total_clusters, int32_t used_clusters,
                             int32_t bytes_per_cluster,
                             int64_t *free_bytes, float *free_percent);

#ifdef __cplusplus
}
#endif
