#include "ean8.h"
#include <stdio.h>

int file_exists(const char *name) {
    FILE *f = fopen(name, "rb");
    if (!f) return 0;
    fclose(f);
    return 1;
}

void usage(const char *p) {
    printf("Uso: %s -i id7 [-o out.pbm] [-s spacing] [-w px_area] [-h height] [-b]\n", p);
    printf(" id7: 7 digitos (zeros a esquerda OK)\n");
}

int main(int argc, char *argv[]) {
    char *id7 = NULL;
    char *out = "barcode.pbm";
    int spacing = 4;
    int px_area = 3;
    int code_height = 50;
    int binario = 0;

    for (int i=1;i<argc;i++) {
        if (strcmp(argv[i], "-i")==0 && i+1<argc) { id7 = argv[++i]; continue; }
        if (strcmp(argv[i], "-o")==0 && i+1<argc) { out = argv[++i]; continue; }
        if (strcmp(argv[i], "-s")==0 && i+1<argc) { spacing = atoi(argv[++i]); continue; }
        if (strcmp(argv[i], "-w")==0 && i+1<argc) { px_area = atoi(argv[++i]); continue; }
        if (strcmp(argv[i], "-h")==0 && i+1<argc) { code_height = atoi(argv[++i]); continue; }
        if (strcmp(argv[i], "-b")==0) { binario = 1; continue; }
        if (strcmp(argv[i], "-help")==0 || strcmp(argv[i], "--help")==0) { usage(argv[0]); return 0; }
    }

    if (!id7) { usage(argv[0]); return 1; }
    if ((int)strlen(id7) != 7) { printf("Erro: id precisa ter 7 digitos\n"); return 1; }
    for (int i=0;i<7;i++) if (!isdigit((unsigned char)id7[i])) { printf("Erro: id invalido\n"); return 1; }
    if (px_area <=0 || spacing < 0 || code_height <= 0) { printf("Parametros invalidos\n"); return 1; }

    if (file_exists(out)) {
        printf("Arquivo %s existe. Sobrescrever? (s/n): ", out);
        int c = getchar();
        if (c != 's' && c != 'S') { printf("Abortando\n"); return 1; }
    }

    int dig[8];
    for (int i=0;i<7;i++) dig[i] = id7[i]-'0';
    dig[7] = calcular_digito_verificador(dig);

    int total_areas = 3 + 28 + 5 + 28 + 3; // 67
    int width = spacing*2 + total_areas * px_area;
    int font_block = px_area * 5;
    int height = spacing*2 + code_height + font_block + 2;

    Imagem *img = criar_imagem(width, height);
    if (!img) { printf("Erro: sem memoria\n"); return 1; }

    int cur = spacing;
    int y0 = spacing;
    int y1 = spacing + code_height;

    desenhar_padrao(img, &cur, MARCADOR_START, px_area, y0, y1);
    for (int i=0;i<4;i++) desenhar_padrao(img, &cur, L_TABLE[dig[i]], px_area, y0, y1);
    desenhar_padrao(img, &cur, MARCADOR_CENTER, px_area, y0, y1);
    for (int i=4;i<8;i++) desenhar_padrao(img, &cur, R_TABLE[dig[i]], px_area, y0, y1);
    desenhar_padrao(img, &cur, MARCADOR_END, px_area, y0, y1);

    char idstr[8]; memcpy(idstr, id7, 7); idstr[7] = 0;
    int y_offset = y1 + 2;
    desenhar_digitos_3x5(img, spacing, total_areas, px_area, idstr, y_offset);

    int ok = 0;
    if (binario) ok = salvar_pbm_binario(img, out);
    else ok = salvar_pbm_ascii(img, out);

    if (!ok) { printf("Erro ao salvar arquivo\n"); liberar_imagem(img); return 1; }

    printf("Gerado: %s (%dx%d) id: ", out, width, height);
    for (int i=0;i<8;i++) printf("%d", dig[i]);
    printf("\n");
    liberar_imagem(img);
    return 0;
}
