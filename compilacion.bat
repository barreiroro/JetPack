@REM Compilación y Enlace con librería gráfica.
@cls


cl /nologo /Zi /GR- /EHs /MD %2 -I %1\DESARROLLO\Lib_Graph\ESAT_rev218\include %1\DESARROLLO\Lib_Graph\ESAT_rev218\bin\ESAT.lib   opengl32.lib user32.lib gdi32.lib shell32.lib Ws2_32.lib
@echo  Proceso por lotes finalizado.
@echo -------------------------------