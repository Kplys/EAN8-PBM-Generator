@echo off
setlocal

echo Compilando...
gcc -std=c11 gerador.c ean8.c -o gerador.exe
if errorlevel 1 ( echo Erro ao compilar gerador & exit /b 1 )
gcc -std=c11 extrator.c ean8.c -o extrator.exe
if errorlevel 1 ( echo Erro ao compilar extrator & exit /b 1 )

echo Testes:
set ids=0000001 0012345 1234567 4017072
for %%I in (%ids%) do (
  echo ==== %%I ASCII ====
  gerador.exe -i %%I -o out_%%I.pbm
  echo Extrator: 
  extrator.exe out_%%I.pbm
  echo ==== %%I BIN ====
  gerador.exe -i %%I -o out_%%I_bin.pbm -b
  echo Extrator bin:
  extrator.exe out_%%I_bin.pbm
)
echo Testes completos.
endlocal
pause
