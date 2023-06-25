#ifndef LV_1024_H
#define LV_1024_H

#include "lvgl/lvgl.h"

#define MATRIX_SIZE 4
typedef struct
{
    lv_obj_t obj;
    lv_obj_t *btnm;
    uint16_t score;
    uint16_t map_count;
    uint16_t matrix[MATRIX_SIZE][MATRIX_SIZE];
    char *btnm_map[MATRIX_SIZE * MATRIX_SIZE + MATRIX_SIZE];
    bool game_over;
} lv_1024_t;

lv_obj_t *lv_1024_create(lv_obj_t *parent);

void lv_1024_set_new_game(lv_obj_t *obj);

uint16_t lv_1024_get_best_tile(lv_obj_t *obj);

uint16_t lv_1024_get_score(lv_obj_t *obj);

bool lv_1024_get_status(lv_obj_t *obj);

#endif /*LV_1024_H*/
