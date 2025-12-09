#include "ean8.h"

const char* L_TABLE[10] = {
    "0001101","0011001","0010011","0111101","0100011",
    "0110001","0101111","0111011","0110111","0001011"
};
const char* R_TABLE[10] = {
    "1110010","1100110","1101100","1000010","1011100",
    "1001110","1010000","1000100","1001000","1110100"
};

Imagem* criar_imagem(int largura, int altura) {
    Imagem *img = (Imagem*)malloc(sizeof(Imagem));
    if (!img) return NULL;
    img->largura = largura;
    img->altura = altura;
    img->pixels = (unsigned char**)malloc(altura * sizeof(unsigned char*));
    if (!img->pixels) { free(img); return NULL; }
    for (int y = 0; y < altura; y++) {
        img->pixels[y] = (unsigned char*)malloc(largura * sizeof(unsigned char));
        if (!img->pixels[y]) {
            for (int j = 0; j < y; j++) free(img->pixels[j]);
            free(img->pixels); free(img); return NULL;
        }
        memset(img->pixels[y], 0, largura * sizeof(unsigned char));
    }
    return img;
}

void liberar_imagem(Imagem *img) {
    if (!img) return;
    for (int y = 0; y < img->altura; y++) free(img->pixels[y]);
    free(img->pixels);
    free(img);
}

int salvar_pbm_ascii(Imagem *img, const char *nome) {
    FILE *f = fopen(nome, "w");
    if (!f) return 0;
    fprintf(f, "P1\n%d %d\n", img->largura, img->altura);
    for (int y = 0; y < img->altura; y++) {
        for (int x = 0; x < img->largura; x++) {
            fprintf(f, "%d", img->pixels[y][x] ? 1 : 0);
            if (x+1 < img->largura) fputc(' ', f);
        }
        fputc('\n', f);
    }
    fclose(f);
    return 1;
}

int salvar_pbm_binario(Imagem *img, const char *nome) {
    FILE *f = fopen(nome, "wb");
    if (!f) return 0;
    fprintf(f, "P4\n%d %d\n", img->largura, img->altura);
    for (int y = 0; y < img->altura; y++) {
        unsigned char byte = 0;
        int bits = 0;
        for (int x = 0; x < img->largura; x++) {
            byte = (byte << 1) | (img->pixels[y][x] ? 1 : 0);
            bits++;
            if (bits == 8) {
                fwrite(&byte, 1, 1, f);
                byte = 0; bits = 0;
            }
        }
        if (bits > 0) {
            byte <<= (8 - bits);
            fwrite(&byte, 1, 1, f);
        }
    }
    fclose(f);
    return 1;
}

static int read_token(FILE *f, char *buf, int n) {
    int c;
    while (1) {
        c = fgetc(f);
        if (c == EOF) return 0;
        if (isspace(c)) continue;
        if (c == '#') {
            while ((c = fgetc(f)) != EOF && c != '\n');
            continue;
        }
        break;
    }
    int i = 0;
    while (c != EOF && !isspace(c) && c != '#') {
        if (i+1 < n) buf[i++] = (char)c;
        c = fgetc(f);
    }
    buf[i] = '\0';
    if (c == '#') ungetc(c, f);
    return 1;
}

Imagem* ler_pbm(const char *nome) {
    FILE *f = fopen(nome, "rb");
    if (!f) return NULL;
    char tok[64];
    if (!read_token(f, tok, sizeof(tok))) { fclose(f); return NULL; }
    int is_bin = 0;
    if (strcmp(tok, "P1") == 0) is_bin = 0;
    else if (strcmp(tok, "P4") == 0) is_bin = 1;
    else { fclose(f); return NULL; }
    if (!read_token(f, tok, sizeof(tok))) { fclose(f); return NULL; }
    int largura = atoi(tok);
    if (!read_token(f, tok, sizeof(tok))) { fclose(f); return NULL; }
    int altura = atoi(tok);
    if (largura <= 0 || altura <= 0) { fclose(f); return NULL; }
    Imagem *img = criar_imagem(largura, altura);
    if (!img) { fclose(f); return NULL; }
    if (!is_bin) {
        for (int y=0;y<altura;y++) {
            for (int x=0;x<largura;x++) {
                if (!read_token(f, tok, sizeof(tok))) { liberar_imagem(img); fclose(f); return NULL; }
                img->pixels[y][x] = (tok[0] == '1') ? 1 : 0;
            }
        }
    } else {
        int c = fgetc(f);
        while (c != EOF && (c == '\n' || c == '\r' || c == ' ' || c == '\t')) c = fgetc(f);
        if (c != EOF) ungetc(c, f);
        for (int y=0;y<altura;y++) {
            int bits_read = 0;
            unsigned char byte;
            while (bits_read < largura) {
                if (fread(&byte, 1, 1, f) != 1) { liberar_imagem(img); fclose(f); return NULL; }
                for (int b=7; b>=0 && bits_read < largura; b--) {
                    img->pixels[y][bits_read++] = ((byte >> b) & 1) ? 1 : 0;
                }
            }
        }
    }
    fclose(f);
    return img;
}

int calcular_digito_verificador(int digitos7[7]) {
    int soma = 0;
    for (int i=0;i<7;i++) {
        int peso = (i % 2 == 0) ? 3 : 1;
        soma += digitos7[i] * peso;
    }
    int mod = soma % 10;
    return (10 - mod) % 10;
}

void desenhar_padrao(Imagem *img, int *cursor_x, const char *padrao, int px_por_area, int y_start, int y_end) {
    int len = (int)strlen(padrao);
    for (int k=0;k<len;k++) {
        int cor = padrao[k] - '0';
        for (int sx=0;sx<px_por_area;sx++) {
            int x = *cursor_x + sx;
            if (x < 0 || x >= img->largura) continue;
            for (int y=y_start;y<y_end && y < img->altura; y++) {
                img->pixels[y][x] = cor ? 1 : 0;
            }
        }
        *cursor_x += px_por_area;
    }
}

static const unsigned char FONT3x5[10][5] = {
    {0b111,0b101,0b101,0b101,0b111},
    {0b010,0b110,0b010,0b010,0b111},
    {0b111,0b001,0b111,0b100,0b111},
    {0b111,0b001,0b111,0b001,0b111},
    {0b101,0b101,0b111,0b001,0b001},
    {0b111,0b100,0b111,0b001,0b111},
    {0b111,0b100,0b111,0b101,0b111},
    {0b111,0b001,0b010,0b100,0b100},
    {0b111,0b101,0b111,0b101,0b111},
    {0b111,0b101,0b111,0b001,0b111}
};

void desenhar_digitos_3x5(Imagem *img, int left_x, int total_areas, int px_por_area, const char *id7, int y_offset) {
    int areas_por_dig = 7;
    int px_por_dig = areas_por_dig * px_por_area;
    int cursor = left_x + 3 * px_por_area;
    for (int i=0;i<7;i++) {
        int d = id7[i]-'0';
        int digit_x = cursor + (px_por_dig - 3*px_por_area)/2;
        for (int row=0; row<5; row++) {
            unsigned char pat = FONT3x5[d][row];
            for (int col=0;col<3;col++) {
                int bit = (pat >> (2-col)) & 1;
                if (!bit) continue;
                for (int sx=0;sx<px_por_area;sx++) for (int sy=0;sy<px_por_area;sy++) {
                    int x = digit_x + col * px_por_area + sx;
                    int y = y_offset + row * px_por_area + sy;
                    if (x>=0 && x<img->largura && y>=0 && y<img->altura) img->pixels[y][x] = 1;
                }
            }
        }
        cursor += px_por_dig;
    }
}
