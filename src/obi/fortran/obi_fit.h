// fwMSX -- obi: conta quanto do disco os arquivos ocupam (Fortran). Ver obi_fit.f90.
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// Quantos clusters de `cluster_bytes` os `n` arquivos (tamanhos em sizes) ocupam; um arquivo vazio nao ocupa nenhum. -1 se invalido.
int obi_clusters_needed(const int *sizes, int n, int cluster_bytes);

#ifdef __cplusplus
}
#endif
