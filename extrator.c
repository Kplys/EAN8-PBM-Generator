#include "ean8.h"
#include <stdio.h>
#include <math.h>

typedef struct { int val; int len; int start; } Run;

Run* runs_from_row(unsigned char *row, int w, int *nruns) {
    Run *runs = (Run*)malloc(sizeof(Run) * (w + 2));
    int cnt = 0;
    if (w <= 0) { *nruns = 0; return runs; }
    int cur = row[0];
    int len = 1;
    int start = 0;
    for (int i=1;i<w;i++) {
        if (row[i] == cur) len++;
        else {
            runs[cnt].val = cur; runs[cnt].len = len; runs[cnt].start = start; cnt++;
            cur = row[i]; len = 1; start = i;
        }
    }
    runs[cnt].val = cur; runs[cnt].len = len; runs[cnt].start = start; cnt++;
    *nruns = cnt;
    return runs;
}

int find_marker_by_runs(Run *runs, int nruns, int *run_index, int *px_est) {
    for (int i=0;i+2 < nruns;i++) {
        Run a = runs[i], b = runs[i+1], c = runs[i+2];
        if (a.val==1 && b.val==0 && c.val==1) {
            double mean = (a.len + b.len + c.len)/3.0;
            double maxdiff = fabs(a.len - mean);
            if (fabs(b.len - mean) > maxdiff) maxdiff = fabs(b.len - mean);
            if (fabs(c.len - mean) > maxdiff) maxdiff = fabs(c.len - mean);
            if (maxdiff / (mean + 1e-9) < 0.6) {
                *run_index = i;
                *px_est = (int)(mean + 0.5);
                return 1;
            }
        }
    }
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc != 2) { printf("Uso: %s arquivo.pbm\n", argv[0]); return 1; }
    Imagem *img = ler_pbm(argv[1]);
    if (!img) { printf("Arquivo invalido ou nao existe\n"); return 1; }

    int best_row = 0, best_cnt = -1;
    for (int y=0;y<img->altura;y++) {
        int cnt = 0;
        for (int x=0;x<img->largura;x++) if (img->pixels[y][x]) cnt++;
        if (cnt > best_cnt) { best_cnt = cnt; best_row = y; }
    }

    int nruns;
    Run *runs = runs_from_row(img->pixels[best_row], img->largura, &nruns);

    int run_idx = -1, px_est = -1;
    int found = find_marker_by_runs(runs, nruns, &run_idx, &px_est);
    if (!found) {
        for (int y=0;y<img->altura && !found;y++) {
            if (y == best_row) continue;
            free(runs);
            runs = runs_from_row(img->pixels[y], img->largura, &nruns);
            found = find_marker_by_runs(runs, nruns, &run_idx, &px_est);
            if (found) { best_row = y; break; }
        }
    }

    if (!found) {
        printf("Codigo de barras nao encontrado (runs detect)\n");
        free(runs); liberar_imagem(img); return 1;
    }

    int start_run = run_idx;
    int start_x = runs[start_run].start;
    int cursor = start_x + 3 * px_est;

    int digits[8];
    char pattern[8];
    auto_read7:
    ;
    for (int i=0;i<4;i++) {
        for (int m=0;m<7;m++) {
            int black=0, white=0;
            for (int s=0;s<px_est;s++) {
                int x = cursor + m*px_est + s;
                if (x < 0 || x >= img->largura) continue;
                if (img->pixels[best_row][x]) black++; else white++;
            }
            pattern[m] = (black >= white) ? '1' : '0';
        }
        pattern[7] = '\0';
        cursor += 7 * px_est;
        int decoded = -1;
        for (int d=0;d<10;d++) if (strcmp(pattern, L_TABLE[d])==0) { decoded = d; break; }
        digits[i] = decoded;
        if (decoded == -1) { printf("Erro: padrao L nao reconhecido (%s)\n", pattern); free(runs); liberar_imagem(img); return 1; }
    }

    cursor += 5 * px_est;

    for (int i=4;i<8;i++) {
        for (int m=0;m<7;m++) {
            int black=0, white=0;
            for (int s=0;s<px_est;s++) {
                int x = cursor + m*px_est + s;
                if (x < 0 || x >= img->largura) continue;
                if (img->pixels[best_row][x]) black++; else white++;
            }
            pattern[m] = (black >= white) ? '1' : '0';
        }
        pattern[7] = '\0';
        cursor += 7 * px_est;
        int decoded = -1;
        for (int d=0;d<10;d++) if (strcmp(pattern, R_TABLE[d])==0) { decoded = d; break; }
        digits[i] = decoded;
        if (decoded == -1) { printf("Erro: padrao R nao reconhecido (%s)\n", pattern); free(runs); liberar_imagem(img); return 1; }
    }

    int tmp7[7];
    for (int i=0;i<7;i++) tmp7[i] = digits[i];
    int dv = calcular_digito_verificador(tmp7);
    if (dv != digits[7]) { printf("Erro: digito verificador invalido (extraido=%d calculado=%d)\n", digits[7], dv); free(runs); liberar_imagem(img); return 1; }

    for (int i=0;i<8;i++) printf("%d", digits[i]);
    printf("\n");
    free(runs);
    liberar_imagem(img);
    return 0;
}
