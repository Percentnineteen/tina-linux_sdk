#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>

#include "lvgl/lvgl.h"
#include "lv_drivers/display/sunxifb.h"
#include "lv_drivers/indev/evdev.h"

#include "lv_1024.h"

static lv_style_t rect_style;
static lv_obj_t *rect_obj;
static lv_obj_t *canvas;

static void game_1024_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj_1024 = lv_event_get_target(e);
    lv_obj_t *label = lv_event_get_user_data(e);

    if (code == LV_EVENT_VALUE_CHANGED)
    {
        if (lv_1024_get_best_tile(obj_1024) >= 1024)
            lv_label_set_text(label, "#00b329 YOU WIN! #");
        else if (lv_1024_get_status(obj_1024))
            lv_label_set_text(label, "#ff0000 GAME OVER! #");
        else
            lv_label_set_text_fmt(label, "SCORE: %d", lv_1024_get_score(obj_1024));
    }
}

static void new_game_btn_event_handler(lv_event_t *e)
{
    lv_obj_t *obj_1024 = lv_event_get_user_data(e);
    lv_1024_set_new_game(obj_1024);
}

void lv_1024_main(void)
{
    /*Create 1024 game*/
    lv_obj_t *obj_1024 = lv_1024_create(lv_scr_act());
    lv_obj_set_style_text_font(obj_1024, &lv_font_montserrat_40, 0);
    lv_obj_set_size(obj_1024, 350, 350);
    lv_obj_center(obj_1024);

    /*Information*/
    lv_obj_t *label = lv_label_create(lv_scr_act());
    lv_label_set_recolor(label, true);
    lv_label_set_text_fmt(label, "SCORE: %d", lv_1024_get_score(obj_1024));
    lv_obj_align_to(label, obj_1024, LV_ALIGN_OUT_TOP_RIGHT, 0, -10);

    lv_obj_add_event_cb(obj_1024, game_1024_event_cb, LV_EVENT_ALL, label);

    /*New Game*/
    lv_obj_t *btn = lv_btn_create(lv_scr_act());
    lv_obj_align_to(btn, obj_1024, LV_ALIGN_OUT_TOP_LEFT, 0, -10);
    lv_obj_add_event_cb(btn, new_game_btn_event_handler, LV_EVENT_CLICKED, obj_1024);

    label = lv_label_create(btn);
    lv_label_set_text(label, "New Game");
    lv_obj_center(label);
}

int main(int argc, char *argv[])
{
    lv_disp_drv_t disp_drv;
    lv_disp_draw_buf_t disp_buf;
    lv_indev_drv_t indev_drv;
    uint32_t rotated = LV_DISP_ROT_NONE;

    lv_disp_drv_init(&disp_drv);

    if (argc >= 2 && atoi(argv[1]) >= 0 && atoi(argv[1]) <= 4)
    {
        rotated = atoi(argv[1]);
#ifndef USE_SUNXIFB_G2D_ROTATE
        if (rotated != LV_DISP_ROT_NONE)
            disp_drv.sw_rotate = 1;
#endif
    }

    /*LittlevGL init*/
    lv_init();

    /*Linux frame buffer device init*/
    sunxifb_init(rotated);

    /*A buffer for LittlevGL to draw the screen's content*/
    static uint32_t width, height;
    sunxifb_get_sizes(&width, &height);

    static lv_color_t *buf;
    buf = (lv_color_t *)sunxifb_alloc(width * height * sizeof(lv_color_t), "lv_1024");

    if (buf == NULL)
    {
        sunxifb_exit();
        printf("malloc draw buffer fail\n");
        return 0;
    }

    /*Initialize a descriptor for the buffer*/
    lv_disp_draw_buf_init(&disp_buf, buf, NULL, width * height);

    /*Initialize and register a display driver*/
    disp_drv.draw_buf = &disp_buf;
    disp_drv.flush_cb = sunxifb_flush;
    disp_drv.hor_res = width;
    disp_drv.ver_res = height;
    disp_drv.rotated = rotated;
    disp_drv.screen_transp = 0;
    lv_disp_drv_register(&disp_drv);

    evdev_init();
    lv_indev_drv_init(&indev_drv);          /*Basic initialization*/
    indev_drv.type = LV_INDEV_TYPE_POINTER; /*See below.*/
    indev_drv.read_cb = evdev_read;         /*See below.*/
    /*Register the driver in LVGL and save the created input device object*/
    lv_indev_t *evdev_indev = lv_indev_drv_register(&indev_drv);

    lv_1024_main();

    /*Handle LitlevGL tasks (tickless mode)*/
    while (1)
    {
        lv_task_handler();
        usleep(1000);
    }

    return 0;
}

/*Set in lv_conf.h as `LV_TICK_CUSTOM_SYS_TIME_EXPR`*/
uint32_t custom_tick_get(void)
{
    static uint64_t start_ms = 0;
    if (start_ms == 0)
    {
        struct timeval tv_start;
        gettimeofday(&tv_start, NULL);
        start_ms = ((uint64_t)tv_start.tv_sec * 1000000 + (uint64_t)tv_start.tv_usec) / 1000;
    }

    struct timeval tv_now;
    gettimeofday(&tv_now, NULL);
    uint64_t now_ms;
    now_ms = ((uint64_t)tv_now.tv_sec * 1000000 + (uint64_t)tv_now.tv_usec) / 1000;

    uint32_t time_ms = now_ms - start_ms;
    return time_ms;
}
