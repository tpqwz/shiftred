/* shiftred.c - Portable C rewrite of the redshifter bash wrapper
 *
 * Single-file, TCC-friendly, aims for CERT C Secure Coding Standard
 * compliance (SEI CERT C, as many rules as practical in a small utility).
 *
 * Original script: https://github.com/wizetek/bash/blob/master/redshifter
 * (Tom Wizetek, GPL)
 *
 * Build (with Tiny C Compiler):
 *   tcc -std=c99 -O2 -o shiftred shiftred.c -lm
 *
 * Or static portable binary:
 *   gcc -std=c99 -O2 -static -o shiftred shiftred.c -lm
 */

#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <limits.h>

/* CERT DCL00-C: avoid magic numbers */
#define CONF_DIR     ".config"
#define PROG_NAME    "shiftred"
#define REDSHIFT_BIN "/usr/bin/redshift"
#define TEMP_STEP    100
#define BRIGHT_STEP  0.02
#define MIN_TEMP     1000
#define MAX_TEMP     25000
#define MIN_BRIGHT   0.1
#define MAX_BRIGHT   1.0
#define PRESET_COUNT 8
#define MAX_PATH     4096
#define BUF_SIZE     128

static const int presets[PRESET_COUNT] = {
    6500, 5500, 4200, 3400, 2700, 2300, 1900, 1200
};

/* CERT STR31-C / STR07-C style bounds-checked path construction */
static int build_conf_path(char *buf, size_t buflen,
                           const char *home, const char *prog)
{
    if (home == NULL || prog == NULL || buflen == 0) return -1;
    int n = snprintf(buf, buflen, "%s/%s/%s", home, CONF_DIR, prog);
    if (n < 0 || (size_t)n >= buflen) return -1;
    return 0;
}

/* Load simple key=value config. Invalid values are ignored (CERT ERR33-C). */
static void load_config(const char *path, int *temp, double *bright)
{
    *temp   = MAX_TEMP;
    *bright = MAX_BRIGHT;

    FILE *f = fopen(path, "r");
    if (f == NULL) return;

    char line[BUF_SIZE];
    while (fgets(line, sizeof(line), f) != NULL) {
        if (strncmp(line, "colortemp=", 10) == 0) {
            char *end = NULL;
            long v = strtol(line + 10, &end, 10);
            if (end != line + 10 && v >= MIN_TEMP && v <= MAX_TEMP)
                *temp = (int)v;
        } else if (strncmp(line, "brightness=", 11) == 0) {
            char *end = NULL;
            double v = strtod(line + 11, &end);
            if (end != line + 11 && v >= MIN_BRIGHT && v <= MAX_BRIGHT)
                *bright = v;
        }
    }
    fclose(f);
}

static int save_config(const char *path, int temp, double bright)
{
    FILE *f = fopen(path, "w");
    if (f == NULL) return -1;
    /* %.2f matches typical bc output precision */
    int rc = fprintf(f, "colortemp=%d\nbrightness=%.2f\n", temp, bright);
    if (rc < 0 || fclose(f) != 0) return -1;
    return 0;
}

/* CERT ENV33-C: avoid system() with shell. Use fork/exec. */
static int run_redshift(int temp, double bright)
{
    pid_t pid = fork();
    if (pid < 0) return -1;

    if (pid == 0) {
        char tbuf[32];
        char bbuf[32];
        if (snprintf(tbuf, sizeof(tbuf), "%d", temp) >= (int)sizeof(tbuf))
            _exit(1);
        if (snprintf(bbuf, sizeof(bbuf), "%.2f", bright) >= (int)sizeof(bbuf))
            _exit(1);
        execl(REDSHIFT_BIN, "redshift", "-P", "-O", tbuf, "-b", bbuf, (char *)NULL);
        _exit(127);
    }

    int status = 0;
    if (waitpid(pid, &status, 0) < 0) return -1;
    return WIFEXITED(status) ? WEXITSTATUS(status) : -1;
}

/* Replicate original cycling logic (descending presets). */
static int cycle_temp(int current)
{
    for (int i = 0; i < PRESET_COUNT; i++) {
        if (presets[i] < current)
            return presets[i];
        if (current == presets[PRESET_COUNT - 1])
            return presets[PRESET_COUNT - 1];
    }
    return presets[PRESET_COUNT - 1];
}

static void show_help(const char *prog)
{
    printf("\n  %s [colortemp] [brightness]\n\n", prog);
    printf("no argument = cycle through presets:\n");
    printf("\t\t6500K = Daylight\n");
    printf("\t\t5500K = Sunlight\n");
    printf("\t\t4200K = Fluorescent\n");
    printf("\t\t3400K = Halogen\n");
    printf("\t\t2700K = Incandescent\n");
    printf("\t\t2300K = Dim Incandescent\n");
    printf("\t\t1900K = Candle\n");
    printf("\t\t1200K = Ember\n\n");
    printf("[colortemp]\n");
    printf("  -\t\tdecrease color temperature by 100K\n");
    printf("  +\t\tincrease color temperature by 100K\n");
    printf("  1000-25000\tset specific color temperature\n");
    printf("  =\t\tno change / restore saved values (e.g. for startup scripts)\n");
    printf("  /\t\treset to default\n\n");
    printf("[brightness]\n");
    printf("  -\t\tdecrease brightness by 0.02\n");
    printf("  +\t\tincrease brightness by 0.02\n");
    printf("  0.1-1.0\tset specific brightness value\n");
    printf("  /\t\treset to default\n\n");
}

int main(int argc, char **argv)
{
    const char *home = getenv("HOME");
    if (home == NULL) {
        fprintf(stderr, "error: HOME not set\n");
        return 1;
    }

    char conf_path[MAX_PATH];
    const char *base = PROG_NAME;
    if (argc > 0 && argv[0] != NULL) {
        const char *slash = strrchr(argv[0], '/');
        base = (slash != NULL) ? slash + 1 : argv[0];
    }
    if (build_conf_path(conf_path, sizeof(conf_path), home, base) != 0) {
        fprintf(stderr, "error: configuration path too long\n");
        return 1;
    }

    int set_temp;
    double set_bright;
    load_config(conf_path, &set_temp, &set_bright);

    /* argv[2] = brightness */
    if (argc > 2 && argv[2] != NULL) {
        if (strcmp(argv[2], "-") == 0) {
            if (set_bright > MIN_BRIGHT) set_bright -= BRIGHT_STEP;
            if (set_bright < MIN_BRIGHT) set_bright = MIN_BRIGHT;
        } else if (strcmp(argv[2], "+") == 0) {
            if (set_bright < MAX_BRIGHT) set_bright += BRIGHT_STEP;
            if (set_bright > MAX_BRIGHT) set_bright = MAX_BRIGHT;
        } else if (argv[2][0] == '/' && argv[2][1] == '\0') {
            set_bright = MAX_BRIGHT;
        } else {
            char *end = NULL;
            double v = strtod(argv[2], &end);
            if (end != argv[2] && *end == '\0' &&
                v >= MIN_BRIGHT && v <= MAX_BRIGHT) {
                set_bright = v;
            }
        }
    }

    /* argv[1] = color temperature */
    if (argc > 1 && argv[1] != NULL && argv[1][0] != '\0') {
        if (strcmp(argv[1], "=") == 0) {
            /* no-op: keep loaded values */
        } else if (strcmp(argv[1], "-") == 0) {
            if (set_temp > MIN_TEMP) set_temp -= TEMP_STEP;
            if (set_temp < MIN_TEMP) set_temp = MIN_TEMP;
        } else if (strcmp(argv[1], "+") == 0) {
            if (set_temp < MAX_TEMP) set_temp += TEMP_STEP;
            if (set_temp > MAX_TEMP) set_temp = MAX_TEMP;
        } else if (argv[1][0] == '/' && argv[1][1] == '\0') {
            set_temp = presets[0];
        } else if (argv[1][0] >= '0' && argv[1][0] <= '9') {
            char *end = NULL;
            long v = strtol(argv[1], &end, 10);
            if (end != argv[1] && *end == '\0' &&
                v >= MIN_TEMP && v <= MAX_TEMP) {
                set_temp = (int)v;
            } else {
                show_help(base);
                return 1;
            }
        } else {
            show_help(base);
            return 1;
        }
    } else {
        /* no argument → cycle presets */
        set_temp = cycle_temp(set_temp);
    }

    /* Final clamps (CERT INT30-C / INT31-C spirit) */
    if (set_temp < MIN_TEMP) set_temp = MIN_TEMP;
    if (set_temp > MAX_TEMP) set_temp = MAX_TEMP;
    if (set_bright < MIN_BRIGHT) set_bright = MIN_BRIGHT;
    if (set_bright > MAX_BRIGHT) set_bright = MAX_BRIGHT;

    int rc = run_redshift(set_temp, set_bright);
    if (rc != 0) {
        fprintf(stderr, "redshift failed or not found (exit %d)\n", rc);
    }

    if (save_config(conf_path, set_temp, set_bright) != 0) {
        fprintf(stderr, "warning: failed to save config to %s\n", conf_path);
    }

    return (rc == 0) ? 0 : 1;
}
