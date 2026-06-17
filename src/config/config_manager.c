#include "config_manager.h"
#include "../../include/vlc.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

vlc_config_t *vlc_config_create(void) {
    vlc_config_t *cfg = calloc(1, sizeof(vlc_config_t));
    if (cfg) pthread_mutex_init(&cfg->mutex, NULL);
    return cfg;
}

void vlc_config_destroy(vlc_config_t *cfg) {
    if (!cfg) return;
    config_entry_t *e = cfg->entries;
    while (e) {
        config_entry_t *next = e->next;
        free(e->key);
        free(e);
        e = next;
    }
    pthread_mutex_destroy(&cfg->mutex);
    free(cfg);
}

int vlc_config_get_int(vlc_config_t *cfg, const char *key, int default_val) {
    if (!cfg || !key) return default_val;
    pthread_mutex_lock(&cfg->mutex);
    config_entry_t *e = cfg->entries;
    while (e) {
        if (strcmp(e->key, key) == 0) {
            int val = e->value;
            pthread_mutex_unlock(&cfg->mutex);
            return val;
        }
        e = e->next;
    }
    pthread_mutex_unlock(&cfg->mutex);
    return default_val;
}

int vlc_config_set_int(vlc_config_t *cfg, const char *key, int value) {
    if (!cfg || !key) return VLC_ERROR_INVALID_PARAM;
    pthread_mutex_lock(&cfg->mutex);
    config_entry_t *e = cfg->entries;
    while (e) {
        if (strcmp(e->key, key) == 0) {
            e->value = value;
            pthread_mutex_unlock(&cfg->mutex);
            return VLC_SUCCESS;
        }
        e = e->next;
    }
    /* Add new entry */
    config_entry_t *new_e = calloc(1, sizeof(config_entry_t));
    if (!new_e) { pthread_mutex_unlock(&cfg->mutex); return VLC_ERROR_NO_MEMORY; }
    new_e->key = strdup(key);
    new_e->value = value;
    new_e->next = cfg->entries;
    cfg->entries = new_e;
    cfg->count++;
    pthread_mutex_unlock(&cfg->mutex);
    return VLC_SUCCESS;
}

int vlc_config_load(vlc_config_t *cfg, const char *path) {
    if (!cfg || !path) return VLC_ERROR_INVALID_PARAM;
    FILE *fp = fopen(path, "r");
    if (!fp) return VLC_SUCCESS;  /* Not an error if file doesn't exist */
    char line[256];
    while (fgets(line, sizeof(line), fp)) {
        char key[128]; int val;
        if (sscanf(line, "%127[^=]=%d", key, &val) == 2) {
            /* Remove trailing newline from key */
            char *k = key + strlen(key) - 1;
            while (k > key && (*k == ' ' || *k == '\n' || *k == '\r')) *k-- = '\0';
            vlc_config_set_int(cfg, key, val);
        }
    }
    fclose(fp);
    return VLC_SUCCESS;
}

int vlc_config_save(vlc_config_t *cfg, const char *path) {
    if (!cfg || !path) return VLC_ERROR_INVALID_PARAM;
    FILE *fp = fopen(path, "w");
    if (!fp) return VLC_ERROR_GENERIC;
    pthread_mutex_lock(&cfg->mutex);
    config_entry_t *e = cfg->entries;
    while (e) {
        fprintf(fp, "%s=%d\n", e->key, e->value);
        e = e->next;
    }
    pthread_mutex_unlock(&cfg->mutex);
    fclose(fp);
    return VLC_SUCCESS;
}