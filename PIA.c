/*
 * PIA — Promedios de calificaciones
 * Versión refactorizada del PIA original (UANL, FCFM, 2020-2021).
 *
 * Cambios principales respecto al original:
 *   1. Parser de CSV con strtok_r en vez de recorrer la cadena carácter por carácter.
 *   2. Calificaciones guardadas como int, no como char[10] convertido con atoi en cada uso.
 *   3. Sin números mágicos: NUM_PARCIALES, APROBATORIA y MAX_ALUMNOS son constantes.
 *   4. Sin rutas absolutas: entrada y salida se pasan por argumento.
 *   5. Manejo de errores de archivo y de líneas mal formadas.
 *   6. Sin fflush(stdin) (comportamiento indefinido).
 *   7. Sin memoria sin inicializar ni strcat sobre buffers vacíos.
 *
 * Compilar:  make
 * Ejecutar:  ./pia [entrada.csv] [salida.csv]
 */

#define _POSIX_C_SOURCE 200809L

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_ALUMNOS   200
#define NUM_PARCIALES 7
#define MAX_LINEA     1024
#define APROBATORIA   7

typedef struct {
    int  numero;
    char matricula[16];
    int  calificacion[NUM_PARCIALES];
} Alumno;

/* ---------- utilidades de cadena ---------- */

static void quitar_salto(char *s)
{
    size_t n = strlen(s);
    while (n > 0 && (s[n - 1] == '\n' || s[n - 1] == '\r')) {
        s[--n] = '\0';
    }
}

static void recortar(char *s)
{
    char  *inicio = s;
    size_t n;

    while (*inicio != '\0' && isspace((unsigned char)*inicio)) {
        inicio++;
    }
    if (inicio != s) {
        memmove(s, inicio, strlen(inicio) + 1);
    }
    n = strlen(s);
    while (n > 0 && isspace((unsigned char)s[n - 1])) {
        s[--n] = '\0';
    }
}

static int convertir_entero(const char *texto, int *destino)
{
    char *fin = NULL;
    long  valor;

    if (texto == NULL || *texto == '\0') {
        return 0;
    }

    errno = 0;
    valor = strtol(texto, &fin, 10);
    if (errno != 0 || fin == texto) {
        return 0;
    }
    while (*fin != '\0' && isspace((unsigned char)*fin)) {
        fin++;
    }
    if (*fin != '\0') {
        return 0;
    }

    if (valor < 0) {
        valor = 0;
    }
    if (valor > 100) {
        valor = 100;
    }
    *destino = (int)valor;
    return 1;
}

/* ---------- lectura del CSV ---------- */

/*
 * Formato esperado por línea:
 *   numero,matricula,c1,c2,c3,c4,c5,c6,c7
 * o bien, sin número de renglón:
 *   matricula,c1,c2,c3,c4,c5,c6,c7
 *
 * La primera línea (encabezado) se descarta automáticamente porque
 * sus campos no son numéricos.
 */
static int parsear_linea(char *linea, Alumno *alumno)
{
    char *campos[32];
    char *guardado = NULL;
    char *token;
    int   total = 0;
    int   desplazamiento;
    int   i;

    token = strtok_r(linea, ",", &guardado);
    while (token != NULL && total < (int)(sizeof campos / sizeof campos[0])) {
        recortar(token);
        campos[total++] = token;
        token = strtok_r(NULL, ",", &guardado);
    }

    if (total >= NUM_PARCIALES + 2) {
        desplazamiento = 2;          /* numero + matricula + notas */
    } else if (total >= NUM_PARCIALES + 1) {
        desplazamiento = 1;          /* matricula + notas */
    } else {
        return 0;                    /* línea incompleta: se ignora */
    }

    memset(alumno, 0, sizeof *alumno);

    if (desplazamiento == 2) {
        alumno->numero = atoi(campos[0]);
    }
    snprintf(alumno->matricula, sizeof alumno->matricula, "%.15s",
             campos[desplazamiento - 1]);

    for (i = 0; i < NUM_PARCIALES; i++) {
        if (!convertir_entero(campos[desplazamiento + i],
                              &alumno->calificacion[i])) {
            alumno->calificacion[i] = 0;
        }
    }
    return 1;
}

static int leer_alumnos(const char *ruta, Alumno alumnos[], int maximo)
{
    FILE *archivo;
    char  linea[MAX_LINEA];
    int   total = 0;

    archivo = fopen(ruta, "r");
    if (archivo == NULL) {
        fprintf(stderr, "No se pudo abrir '%s': %s\n", ruta, strerror(errno));
        return -1;
    }

    while (fgets(linea, sizeof linea, archivo) != NULL) {
        quitar_salto(linea);
        if (linea[0] == '\0' || linea[0] == '#') {
            continue;
        }
        if (total >= maximo) {
            fprintf(stderr, "Aviso: se alcanzó el máximo de %d alumnos.\n", maximo);
            break;
        }
        if (parsear_linea(linea, &alumnos[total])) {
            total++;
        }
    }

    fclose(archivo);
    return total;
}

/* ---------- cálculos ---------- */

static double promedio_alumno(const Alumno *alumno)
{
    int suma = 0;
    int i;

    for (i = 0; i < NUM_PARCIALES; i++) {
        suma += alumno->calificacion[i];
    }
    return (double)suma / NUM_PARCIALES;
}

static int reprobadas_alumno(const Alumno *alumno)
{
    int contador = 0;
    int i;

    for (i = 0; i < NUM_PARCIALES; i++) {
        if (alumno->calificacion[i] < APROBATORIA) {
            contador++;
        }
    }
    return contador;
}

static double promedio_parcial(const Alumno alumnos[], int total, int parcial)
{
    int suma = 0;
    int i;

    if (total == 0) {
        return 0.0;
    }
    for (i = 0; i < total; i++) {
        suma += alumnos[i].calificacion[parcial];
    }
    return (double)suma / total;
}

static int reprobadas_parcial(const Alumno alumnos[], int total, int parcial)
{
    int contador = 0;
    int i;

    for (i = 0; i < total; i++) {
        if (alumnos[i].calificacion[parcial] < APROBATORIA) {
            contador++;
        }
    }
    return contador;
}

/* ---------- escritura del reporte ---------- */

static int escribir_reporte(const char *ruta, const Alumno alumnos[], int total)
{
    FILE  *archivo;
    double promedio_grupo = 0.0;
    int    total_reprobadas = 0;
    int    i;
    int    p;

    archivo = fopen(ruta, "w");
    if (archivo == NULL) {
        fprintf(stderr, "No se pudo crear '%s': %s\n", ruta, strerror(errno));
        return -1;
    }

    fprintf(archivo, "MATRICULA");
    for (p = 0; p < NUM_PARCIALES; p++) {
        fprintf(archivo, ",P%d", p + 1);
    }
    fprintf(archivo, ",PROMEDIO,REPROBADAS\n");

    for (i = 0; i < total; i++) {
        fprintf(archivo, "%s", alumnos[i].matricula);
        for (p = 0; p < NUM_PARCIALES; p++) {
            fprintf(archivo, ",%d", alumnos[i].calificacion[p]);
        }
        fprintf(archivo, ",%.2f,%d\n",
                promedio_alumno(&alumnos[i]),
                reprobadas_alumno(&alumnos[i]));
    }

    fprintf(archivo, "\nPROMEDIO POR PARCIAL");
    for (p = 0; p < NUM_PARCIALES; p++) {
        double promedio = promedio_parcial(alumnos, total, p);
        promedio_grupo += promedio;
        fprintf(archivo, ",%.2f", promedio);
    }
    promedio_grupo /= NUM_PARCIALES;
    fprintf(archivo, "\n");

    fprintf(archivo, "REPROBADAS POR PARCIAL");
    for (p = 0; p < NUM_PARCIALES; p++) {
        int reprobadas = reprobadas_parcial(alumnos, total, p);
        total_reprobadas += reprobadas;
        fprintf(archivo, ",%d", reprobadas);
    }
    fprintf(archivo, "\n");

    fprintf(archivo, "PROMEDIO DEL GRUPO,%.2f\n", promedio_grupo);
    fprintf(archivo, "TOTAL DE CALIFICACIONES REPROBADAS,%d\n", total_reprobadas);

    fclose(archivo);
    return 0;
}

/* ---------- principal ---------- */

int main(int argc, char *argv[])
{
    Alumno alumnos[MAX_ALUMNOS];
    const char *entrada = (argc > 1) ? argv[1] : "calificaciones.csv";
    const char *salida  = (argc > 2) ? argv[2] : "reporte.csv";
    int total;

    total = leer_alumnos(entrada, alumnos, MAX_ALUMNOS);
    if (total < 0) {
        return EXIT_FAILURE;
    }
    if (total == 0) {
        fprintf(stderr, "No se encontraron alumnos válidos en '%s'.\n", entrada);
        return EXIT_FAILURE;
    }

    if (escribir_reporte(salida, alumnos, total) != 0) {
        return EXIT_FAILURE;
    }

    printf("Procesados %d alumnos. Reporte escrito en '%s'.\n", total, salida);
    return EXIT_SUCCESS;
}
