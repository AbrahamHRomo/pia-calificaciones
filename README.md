# PIA — Promedios de calificaciones (versión refactorizada)

Programa en C que lee un archivo CSV con calificaciones de alumnos, calcula promedios
y genera un reporte.

Esta es una reescritura del PIA original del curso de **Programación Estructurada**
(UANL, FCFM, semestre 2020-2021). El objetivo no es cambiar lo que hace, sino mostrar
el mismo programa con criterio de ingeniería: legible, sin números mágicos, sin
comportamiento indefinido y compilable sin advertencias.

## Compilar

```sh
make
```

## Ejecutar

```sh
./pia ejemplo_calificaciones.csv reporte.csv
```

O directamente:

```sh
make run
```

## Formato de entrada

```
numero,matricula,c1,c2,c3,c4,c5,c6,c7
```

Ejemplo:

```
01,1234,9,8,10,7,6,9,8
02,1235,7,7,7,7,7,7,7
```

También acepta líneas sin número de renglón:

```
1234,9,8,10,7,6,9,8
```

La primera línea se descarta automáticamente si es un encabezado, porque sus campos
no son numéricos. Las líneas incompletas se ignoran.

## Formato de salida

```
MATRICULA,P1,P2,P3,P4,P5,P6,P7,PROMEDIO,REPROBADAS
1234,9,8,10,7,6,9,8,8.14,1
...

PROMEDIO POR PARCIAL,8.00,7.50,...
REPROBADAS POR PARCIAL,2,3,...
PROMEDIO DEL GRUPO,7.85
TOTAL DE CALIFICACIONES REPROBADAS,17
```

## Qué cambió respecto al original

| Original | Refactor |
|---|---|
| Recorrido manual de la cadena, índice por índice | `strtok_r` separando por comas |
| Calificaciones como `char[10]` + `atoi` en cada uso | `int calificacion[7]` |
| `/ 29` y `50` hardcodeados | `NUM_PARCIALES`, `MAX_ALUMNOS` |
| Ruta absoluta `/home/radical-ab/...` | Argumentos de línea de comandos |
| `strncpy` sin terminador `\0` | `snprintf` con tamaño |
| `strcat` sobre buffer sin inicializar | `memset` + `snprintf` |
| `fflush(stdin)` | Eliminado (era comportamiento indefinido) |
| `case 6` leyendo `parcial1` | Bucle sobre el arreglo |
| Sin verificación de `fopen` | Manejo de error con `strerror` |
| 7 bloques de código casi idénticos | Un solo bucle |
| Sin compilar limpio | `-Wall -Wextra -Wpedantic` sin advertencias |

## Estructura

```
PIA.c                        programa completo
Makefile                     compilación
ejemplo_calificaciones.csv   datos de prueba
README.md                    este archivo
```

## Autor

Abraham Hernández Romo — [github.com/AbrahamHRomo](https://github.com/AbrahamHRomo)
