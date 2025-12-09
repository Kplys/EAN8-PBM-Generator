#ifndef EAN8_H
#define EAN8_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MARCADOR_START "101"
#define MARCADOR_CENTER "01010"
#define MARCADOR_END "101"

typedef struct {
    int largura;
    int altura;
    unsigned char **pixels;
} Imagem;

Imagem* criar_imagem(int largura, int altura);
void liberar_imagem(Imagem *img);
int salvar_pbm_ascii(Imagem *img, const char *nome);
int salvar_pbm_binario(Imagem *img, const char *nome);
Imagem* ler_pbm(const char *nome);
int calcular_digito_verificador(int digitos7[7]);
void desenhar_padrao(Imagem *img, int *cursor_x, const char *padrao, int px_por_area, int y_start, int y_end);
void desenhar_digitos_3x5(Imagem *img, int left_x, int total_areas, int px_por_area, const char *id7, int y_offset);

extern const char* L_TABLE[10];
extern const char* R_TABLE[10];

#endif
