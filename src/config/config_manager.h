#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include <pthread.h>

typedef struct config_entry {
    char *key;
    int value;
    struct config_entry *next;
} config_entry_t;

typedef struct vlc_config {
    config_entry_t *entries;
    int count;
    pthread_mutex_t mutex;
} vlc_config_t;

vlc_config_t *vlc_config_create(void);
void vlc_config_destroy(vlc_config_t *config);
int vlc_config_load(vlc_config_t *config, const char *path);
int vlc_config_save(vlc_config_t *config, const char *path);
int vlc_config_get_int(vlc_config_t *config, const char *key, int default_val);
int vlc_config_set_int(vlc_config_t *config, const char *key, int value);

#endif