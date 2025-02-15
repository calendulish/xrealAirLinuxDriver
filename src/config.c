#include "config.h"
#include "plugins.h"
#include "string.h"

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>

const char *joystick_output_mode = "joystick";
const char *mouse_output_mode = "mouse";
const char *external_only_output_mode = "external_only";

driver_config_type *default_config() {
    driver_config_type *config = malloc(sizeof(driver_config_type));
    if (config == NULL) {
        fprintf(stderr, "Error allocating config");
        exit(1);
    }

    config->disabled = true;
    config->use_roll_axis = false;
    config->mouse_sensitivity = 30;
    config->output_mode = strdup(mouse_output_mode);

    config->debug_threads = false;
    config->debug_joystick = false;
    config->debug_multi_tap = false;
    config->debug_ipc = false;

    return config;
}

void update_config(driver_config_type **config, driver_config_type *new_config) {
    free((*config)->output_mode);
    free(*config);
    *config = new_config;
}

bool equal(char *key, const char *desired_key) {
    return strcmp(key, desired_key) == 0;
}

void boolean_config(char* key, char *value, bool *config_value) {
    *config_value = equal(value, "true");
}

void float_config(char* key, char *value, float *config_value) {
    char *endptr;
    errno = 0;
    float num = strtof(value, &endptr);
    if (errno != ERANGE && endptr != value) {
        *config_value = num;
    } else {
        fprintf(stderr, "Error parsing %s value: %s\n", key, value);
    }
}

void int_config(char* key, char *value, int *config_value) {
    char *endptr;
    errno = 0;
    long num = strtol(value, &endptr, 10);
    if (errno != ERANGE && endptr != value) {
        *config_value = (int) num;
    } else {
        fprintf(stderr, "Error parsing %s value: %s\n", key, value);
    }
}

void string_config(char* key, char *value, char **config_value) {
    free_and_clear(config_value);
    *config_value = strdup(value);
}

driver_config_type* parse_config_file(FILE *fp) {
    driver_config_type *config = default_config();
    void *plugin_configs = plugins.default_config();

    char line[1024];
    while (fgets(line, sizeof(line), fp) != NULL) {
        char *key = strtok(line, "=");
        char *value = strtok(NULL, "\n");
        if (equal(key, "disabled")) {
            boolean_config(key, value, &config->disabled);
        } else if (equal(key, "debug")) {
            char *token = strtok(value, ",");
            while (token != NULL) {
                if (equal(token, "joystick")) {
                    config->debug_joystick = true;
                }
                if (equal(token, "taps")) {
                    config->debug_multi_tap = true;
                }
                if (equal(token, "threads")) {
                    config->debug_threads = true;
                }
                if (equal(token, "ipc")) {
                    config->debug_ipc = true;
                }
                token = strtok(NULL, ",");
            }
        } else if (equal(key, "use_roll_axis")) {
            config->use_roll_axis = true;
        } else if (equal(key, "mouse_sensitivity")) {
            int_config(key, value, &config->mouse_sensitivity);
        } else if (equal(key, "output_mode")) {
            string_config(key, value, &config->output_mode);
        }

        plugins.handle_config_line(plugin_configs, key, value);
    }
    plugins.set_config(plugin_configs);

    return config;
}

bool is_joystick_mode(driver_config_type *config) {
    return strcmp(config->output_mode, joystick_output_mode) == 0;
}

bool is_mouse_mode(driver_config_type *config) {
    return strcmp(config->output_mode, mouse_output_mode) == 0;
}

bool is_external_mode(driver_config_type *config) {
    return strcmp(config->output_mode, external_only_output_mode) == 0;
}

bool is_evdev_output_mode(driver_config_type *config) {
    return is_mouse_mode(config) || is_joystick_mode(config);
}