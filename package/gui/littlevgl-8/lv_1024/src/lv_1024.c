#include "lv_1024.h"

#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#define LV_1024_BG_COLOR lv_color_hex(0xb3a397)
#define LV_1024_TEXT_BLACK_COLOR lv_color_hex(0x6c635b)
#define LV_1024_TEXT_WHITE_COLOR lv_color_hex(0xf8f5f0)

#define LV_1024_NUMBER_EMPTY_COLOR lv_color_hex(0xc7b9ac)
#define LV_1024_NUMBER_2_COLOR lv_color_hex(0xeee4da)
#define LV_1024_NUMBER_4_COLOR lv_color_hex(0xede0c8)
#define LV_1024_NUMBER_8_COLOR lv_color_hex(0xf2b179)
#define LV_1024_NUMBER_16_COLOR lv_color_hex(0xf59563)
#define LV_1024_NUMBER_32_COLOR lv_color_hex(0xf67c5f)
#define LV_1024_NUMBER_64_COLOR lv_color_hex(0xf75f3b)
#define LV_1024_NUMBER_128_COLOR lv_color_hex(0xedcf72)
#define LV_1024_NUMBER_256_COLOR lv_color_hex(0xedcc61)
#define LV_1024_NUMBER_512_COLOR lv_color_hex(0xedc850)
#define LV_1024_NUMBER_1024_COLOR lv_color_hex(0xedc53f)

static void lv_1024_constructor(const lv_obj_class_t *class_p, lv_obj_t *obj);
static void lv_1024_destructor(const lv_obj_class_t *class_p, lv_obj_t *obj);
static void lv_1024_event(const lv_obj_class_t *class_p, lv_event_t *e);
static void btnm_event_cb(lv_event_t *e);

static void init_matrix_num(u_int16_t matrix[MATRIX_SIZE][MATRIX_SIZE]);
static void new_random_btnm(u_int16_t matrix[MATRIX_SIZE][MATRIX_SIZE]);
static void update_btnm_map(char *lv_1024_btnm_map[], u_int16_t matrix[MATRIX_SIZE][MATRIX_SIZE]);
static lv_color_t get_num_color(u_int16_t num);
static char *int_to_str(char *str, u_int16_t num);

static u_int8_t count_empty(u_int16_t matrix[MATRIX_SIZE][MATRIX_SIZE]);
static u_int8_t find_target(u_int16_t array[MATRIX_SIZE], u_int8_t x, u_int8_t stop);
static void rotate_matrix(u_int16_t matrix[MATRIX_SIZE][MATRIX_SIZE]);
static bool find_pair_down(u_int16_t matrix[MATRIX_SIZE][MATRIX_SIZE]);
static bool slide_array(u_int16_t *score, u_int16_t array[MATRIX_SIZE]);
static bool move_up(u_int16_t *score, u_int16_t matrix[MATRIX_SIZE][MATRIX_SIZE]);
static bool move_down(u_int16_t *score, u_int16_t matrix[MATRIX_SIZE][MATRIX_SIZE]);
static bool move_left(u_int16_t *score, u_int16_t matrix[MATRIX_SIZE][MATRIX_SIZE]);
static bool move_right(u_int16_t *score, u_int16_t matrix[MATRIX_SIZE][MATRIX_SIZE]);
static bool game_over(u_int16_t matrix[MATRIX_SIZE][MATRIX_SIZE]);

const lv_obj_class_t lv_1024_class = {
    .constructor_cb = lv_1024_constructor,
    .destructor_cb = lv_1024_destructor,
    .event_cb = lv_1024_event,
    .width_def = LV_DPI_DEF * 2,
    .height_def = LV_DPI_DEF * 2,
    .group_def = LV_OBJ_CLASS_GROUP_DEF_TRUE,
    .instance_size = sizeof(lv_1024_t),
    .base_class = &lv_obj_class};

lv_obj_t *lv_1024_create(lv_obj_t *parent)
{
    LV_LOG_INFO("begin");
    lv_obj_t *obj = lv_obj_class_create_obj(&lv_1024_class, parent);
    lv_obj_class_init_obj(obj);
    return obj;
}

void lv_1024_set_new_game(lv_obj_t *obj)
{
    LV_ASSERT_OBJ(obj, &lv_1024_class);

    lv_1024_t *game_1024 = (lv_1024_t *)obj;

    game_1024->score = 0;
    game_1024->game_over = false;
    game_1024->map_count = MATRIX_SIZE * MATRIX_SIZE + MATRIX_SIZE;

    init_matrix_num(game_1024->matrix);
    update_btnm_map(game_1024->btnm_map, game_1024->matrix);
    lv_btnmatrix_set_map(game_1024->btnm, game_1024->btnm_map);

    lv_event_send(obj, LV_EVENT_VALUE_CHANGED, NULL);
}

u_int16_t lv_1024_get_score(lv_obj_t *obj)
{
    LV_ASSERT_OBJ(obj, &lv_1024_class);

    lv_1024_t *game_1024 = (lv_1024_t *)obj;

    return game_1024->score;
}

bool lv_1024_get_status(lv_obj_t *obj)
{
    LV_ASSERT_OBJ(obj, &lv_1024_class);

    lv_1024_t *game_1024 = (lv_1024_t *)obj;

    return game_1024->game_over;
}

u_int16_t lv_1024_get_best_tile(lv_obj_t *obj)
{
    LV_ASSERT_OBJ(obj, &lv_1024_class);

    lv_1024_t *game_1024 = (lv_1024_t *)obj;

    u_int8_t x, y;
    u_int16_t best_tile = 0;

    for (x = 0; x < MATRIX_SIZE; x++)
    {
        for (y = 0; y < MATRIX_SIZE; y++)
        {
            if (best_tile < game_1024->matrix[x][y])
                best_tile = game_1024->matrix[x][y];
        }
    }

    return (u_int16_t)(1 << best_tile);
}

static void lv_1024_constructor(const lv_obj_class_t *class_p, lv_obj_t *obj)
{
    LV_UNUSED(class_p);
    LV_TRACE_OBJ_CREATE("begin");

    lv_1024_t *game_1024 = (lv_1024_t *)obj;

    game_1024->score = 0;
    game_1024->game_over = false;
    game_1024->map_count = MATRIX_SIZE * MATRIX_SIZE + MATRIX_SIZE;

    u_int16_t index;
    for (index = 0; index < game_1024->map_count; index++)
    {

        if (((index + 1) % 5) == 0)
        {
            game_1024->btnm_map[index] = lv_mem_alloc(2);
            if ((index + 1) == game_1024->map_count)
                strcpy(game_1024->btnm_map[index], "");
            else
                strcpy(game_1024->btnm_map[index], "\n");
        }
        else
        {
            game_1024->btnm_map[index] = lv_mem_alloc(5);
            strcpy(game_1024->btnm_map[index], " ");
        }
    }

    init_matrix_num(game_1024->matrix);
    update_btnm_map(game_1024->btnm_map, game_1024->matrix);

    /*obj style init*/
    lv_theme_t *theme = lv_theme_get_from_obj(obj);
    lv_obj_set_style_outline_color(obj, theme->color_primary, LV_STATE_FOCUS_KEY);
    lv_obj_set_style_outline_width(obj, lv_disp_dpx(theme->disp, 2), LV_STATE_FOCUS_KEY);
    lv_obj_set_style_outline_pad(obj, lv_disp_dpx(theme->disp, 2), LV_STATE_FOCUS_KEY);
    lv_obj_set_style_outline_opa(obj, LV_OPA_50, LV_STATE_FOCUS_KEY);

    /*game_1024->btnm init*/
    game_1024->btnm = lv_btnmatrix_create(obj);
    lv_obj_set_size(game_1024->btnm, LV_PCT(100), LV_PCT(100));
    lv_obj_center(game_1024->btnm);
    lv_obj_set_style_pad_all(game_1024->btnm, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(game_1024->btnm, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_group_remove_obj(game_1024->btnm);
    lv_obj_add_flag(game_1024->btnm, LV_OBJ_FLAG_EVENT_BUBBLE);

    lv_btnmatrix_set_map(game_1024->btnm, game_1024->btnm_map);
    lv_btnmatrix_set_btn_ctrl_all(game_1024->btnm, LV_BTNMATRIX_CTRL_DISABLED);

    lv_obj_add_event_cb(game_1024->btnm, btnm_event_cb, LV_EVENT_ALL, NULL);

    LV_TRACE_OBJ_CREATE("finished");
}

static void lv_1024_destructor(const lv_obj_class_t *class_p, lv_obj_t *obj)
{
    LV_UNUSED(class_p);

    lv_1024_t *game_1024 = (lv_1024_t *)obj;

    u_int16_t index, count;
    for (index = 0; index < game_1024->map_count; index++)
    {
        lv_mem_free(game_1024->btnm_map[index]);
    }
}

static void lv_1024_event(const lv_obj_class_t *class_p, lv_event_t *e)
{
    LV_UNUSED(class_p);

    lv_res_t res;

    /*Call the ancestor's event handler*/
    res = lv_obj_event_base(&lv_1024_class, e);
    if (res != LV_RES_OK)
        return;

    bool success = false;
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *btnm = lv_event_get_target(e);
    lv_obj_t *obj = lv_event_get_current_target(e);

    lv_1024_t *game_1024 = (lv_1024_t *)obj;

    if (code == LV_EVENT_CLICKED)
    {
        game_1024->game_over = game_over(game_1024->matrix);
        if (!game_1024->game_over)
        {
            switch (lv_indev_get_gesture_dir(lv_indev_get_act()))
            {
            case LV_DIR_TOP:
                success = move_left(&(game_1024->score), game_1024->matrix);
                break;
            case LV_DIR_BOTTOM:
                success = move_right(&(game_1024->score), game_1024->matrix);
                break;
            case LV_DIR_LEFT:
                success = move_up(&(game_1024->score), game_1024->matrix);
                break;
            case LV_DIR_RIGHT:
                success = move_down(&(game_1024->score), game_1024->matrix);
                break;
            default:
                break;
            }
        }
        else
        {
            res = lv_event_send(obj, LV_EVENT_VALUE_CHANGED, NULL);
            if (res != LV_RES_OK)
                return;
        }
    }
    else if (code == LV_EVENT_KEY)
    {
        game_1024->game_over = game_over(game_1024->matrix);
        if (!game_1024->game_over)
        {
            switch (*((u_int8_t *)lv_event_get_param(e)))
            {
            case LV_KEY_UP:
                success = move_left(&(game_1024->score), game_1024->matrix);
                break;
            case LV_KEY_DOWN:
                success = move_right(&(game_1024->score), game_1024->matrix);
                break;
            case LV_KEY_LEFT:
                success = move_up(&(game_1024->score), game_1024->matrix);
                break;
            case LV_KEY_RIGHT:
                success = move_down(&(game_1024->score), game_1024->matrix);
                break;
            default:
                break;
            }
        }
        else
        {
            res = lv_event_send(obj, LV_EVENT_VALUE_CHANGED, NULL);
            if (res != LV_RES_OK)
                return;
        }
    }

    if (success)
    {
        new_random_btnm(game_1024->matrix);
        update_btnm_map(game_1024->btnm_map, game_1024->matrix);
        lv_btnmatrix_set_map(game_1024->btnm, game_1024->btnm_map);

        res = lv_event_send(obj, LV_EVENT_VALUE_CHANGED, NULL);
        if (res != LV_RES_OK)
            return;
    }
}

static void btnm_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *btnm = lv_event_get_target(e);
    lv_obj_t *parent = lv_obj_get_parent(btnm);

    lv_1024_t *game_1024 = (lv_1024_t *)parent;

    if (code == LV_EVENT_DRAW_PART_BEGIN)
    {
        lv_obj_draw_part_dsc_t *dsc = lv_event_get_param(e);

        if ((dsc->id >= 0) && (dsc->label_dsc))
        {
            u_int16_t x, y, num;

            x = (u_int16_t)((dsc->id) / 4);
            y = (dsc->id) % 4;
            num = (u_int16_t)(1 << (game_1024->matrix[x][y]));

            dsc->rect_dsc->radius = 3;
            dsc->rect_dsc->bg_color = get_num_color(num);

            if (num < 8)
                dsc->label_dsc->color = LV_1024_TEXT_BLACK_COLOR;
            else
                dsc->label_dsc->color = LV_1024_TEXT_WHITE_COLOR;
        }
        else if ((dsc->id == 0) && !(dsc->label_dsc))
        {
            dsc->rect_dsc->radius = 5;
            dsc->rect_dsc->bg_color = LV_1024_BG_COLOR;
            dsc->rect_dsc->border_width = 0;
        }
    }
}

static void init_matrix_num(u_int16_t matrix[MATRIX_SIZE][MATRIX_SIZE])
{
    u_int8_t x, y;

    for (x = 0; x < MATRIX_SIZE; x++)
    {
        for (y = 0; y < MATRIX_SIZE; y++)
        {
            matrix[x][y] = 0;
        }
    }
    new_random_btnm(matrix);
    new_random_btnm(matrix);
}

static void new_random_btnm(u_int16_t matrix[MATRIX_SIZE][MATRIX_SIZE])
{
    static bool initialized = false;
    u_int16_t x, y;
    u_int16_t r, len = 0;
    u_int16_t n, list[MATRIX_SIZE * MATRIX_SIZE][2];

    if (!initialized)
    {
        srand(time(NULL));
        initialized = true;
    }

    for (x = 0; x < MATRIX_SIZE; x++)
    {
        for (y = 0; y < MATRIX_SIZE; y++)
        {
            if (matrix[x][y] == 0)
            {
                list[len][0] = x;
                list[len][1] = y;
                len++;
            }
        }
    }

    if (len > 0)
    {
        r = rand() % len;
        x = list[r][0];
        y = list[r][1];
        n = ((rand() % 10) / 9) + 1;
        matrix[x][y] = n;
    }
}

static void update_btnm_map(char *btnm_map[], u_int16_t matrix[MATRIX_SIZE][MATRIX_SIZE])
{
    u_int8_t x, y, index;

    index = 0;
    for (x = 0; x < MATRIX_SIZE; x++)
    {
        for (y = 0; y < MATRIX_SIZE; y++)
        {

            if (((index + 1) % 5) == 0)
                index++;

            if (matrix[x][y] != 0)
                int_to_str(btnm_map[index], (u_int16_t)(1 << matrix[x][y]));
            else
                strcpy(btnm_map[index], " ");

            index++;
        }
    }
}

static char *int_to_str(char *str, u_int16_t num)
{
    u_int8_t i = 0;
    if (num < 0)
    {
        num = -num;
        str[i++] = '-';
    }
    do
    {
        str[i++] = num % 10 + 48;
        num /= 10;
    } while (num);
    str[i] = '\0';
    u_int8_t j = 0;
    if (str[0] == '-')
    {
        j = 1;
        ++i;
    }
    for (; j < (i / 2); j++)
    {
        str[j] = str[j] + str[i - 1 - j];
        str[i - 1 - j] = str[j] - str[i - 1 - j];
        str[j] = str[j] - str[i - 1 - j];
    }
    return str;
}

static u_int8_t find_target(u_int16_t array[MATRIX_SIZE], u_int8_t x, u_int8_t stop)
{
    u_int8_t t;
    if (x == 0)
    {
        return x;
    }
    for (t = x - 1;; t--)
    {
        if (array[t] != 0)
        {
            if (array[t] != array[x])
            {
                return t + 1;
            }
            return t;
        }
        else
        {
            if (t == stop)
            {
                return t;
            }
        }
    }
    return x;
}

static bool slide_array(u_int16_t *score, u_int16_t array[MATRIX_SIZE])
{
    bool success = false;
    u_int8_t x, t, stop = 0;

    for (x = 0; x < MATRIX_SIZE; x++)
    {
        if (array[x] != 0)
        {
            t = find_target(array, x, stop);
            if (t != x)
            {
                if (array[t] == 0)
                {
                    array[t] = array[x];
                }
                else if (array[t] == array[x])
                {
                    array[t]++;
                    *score += (uint32_t)1 << array[t];
                    stop = t + 1;
                }
                array[x] = 0;
                success = true;
            }
        }
    }
    return success;
}

static void rotate_matrix(u_int16_t matrix[MATRIX_SIZE][MATRIX_SIZE])
{
    u_int8_t i, j, n = MATRIX_SIZE;
    u_int16_t tmp;

    for (i = 0; i < (n / 2); i++)
    {
        for (j = i; j < (n - i - 1); j++)
        {
            tmp = matrix[i][j];
            matrix[i][j] = matrix[j][n - i - 1];
            matrix[j][n - i - 1] = matrix[n - i - 1][n - j - 1];
            matrix[n - i - 1][n - j - 1] = matrix[n - j - 1][i];
            matrix[n - j - 1][i] = tmp;
        }
    }
}

static bool move_up(u_int16_t *score, u_int16_t matrix[MATRIX_SIZE][MATRIX_SIZE])
{
    bool success = false;
    u_int8_t x;

    for (x = 0; x < MATRIX_SIZE; x++)
    {
        success |= slide_array(score, matrix[x]);
    }
    return success;
}

static bool move_left(u_int16_t *score, u_int16_t matrix[MATRIX_SIZE][MATRIX_SIZE])
{
    bool success;

    rotate_matrix(matrix);

    success = move_up(score, matrix);
    rotate_matrix(matrix);
    rotate_matrix(matrix);
    rotate_matrix(matrix);

    return success;
}

static bool move_down(u_int16_t *score, u_int16_t matrix[MATRIX_SIZE][MATRIX_SIZE])
{
    bool success;

    rotate_matrix(matrix);
    rotate_matrix(matrix);

    success = move_up(score, matrix);
    rotate_matrix(matrix);
    rotate_matrix(matrix);

    return success;
}

static bool move_right(u_int16_t *score, u_int16_t matrix[MATRIX_SIZE][MATRIX_SIZE])
{
    bool success;

    rotate_matrix(matrix);
    rotate_matrix(matrix);
    rotate_matrix(matrix);

    success = move_up(score, matrix);
    rotate_matrix(matrix);

    return success;
}

static bool find_pair_down(u_int16_t matrix[MATRIX_SIZE][MATRIX_SIZE])
{
    bool success = false;
    u_int8_t x, y;

    for (x = 0; x < MATRIX_SIZE; x++)
    {
        for (y = 0; y < (MATRIX_SIZE - 1); y++)
        {
            if (matrix[x][y] == matrix[x][y + 1])
                return true;
        }
    }
    return success;
}

static u_int8_t count_empty(u_int16_t matrix[MATRIX_SIZE][MATRIX_SIZE])
{
    u_int8_t x, y;
    u_int8_t count = 0;

    for (x = 0; x < MATRIX_SIZE; x++)
    {
        for (y = 0; y < MATRIX_SIZE; y++)
        {
            if (matrix[x][y] == 0)
            {
                count++;
            }
        }
    }
    return count;
}

static lv_color_t get_num_color(u_int16_t num)
{
    lv_color_t color;

    switch (num)
    {
    case 0:
        color = LV_1024_NUMBER_EMPTY_COLOR;
        break;
    case 1:
        color = LV_1024_NUMBER_EMPTY_COLOR;
        break;
    case 2:
        color = LV_1024_NUMBER_2_COLOR;
        break;
    case 4:
        color = LV_1024_NUMBER_4_COLOR;
        break;
    case 8:
        color = LV_1024_NUMBER_8_COLOR;
        break;
    case 16:
        color = LV_1024_NUMBER_16_COLOR;
        break;
    case 32:
        color = LV_1024_NUMBER_32_COLOR;
        break;
    case 64:
        color = LV_1024_NUMBER_64_COLOR;
        break;
    case 128:
        color = LV_1024_NUMBER_128_COLOR;
        break;
    case 256:
        color = LV_1024_NUMBER_256_COLOR;
        break;
    case 512:
        color = LV_1024_NUMBER_512_COLOR;
        break;
    case 1024:
        color = LV_1024_NUMBER_1024_COLOR;
        break;
    default:
        color = LV_1024_NUMBER_1024_COLOR;
        break;
    }
    return color;
}

static bool game_over(u_int16_t matrix[MATRIX_SIZE][MATRIX_SIZE])
{
    bool ended = true;

    if (count_empty(matrix) > 0)
        return false;
    if (find_pair_down(matrix))
        return false;

    rotate_matrix(matrix);
    if (find_pair_down(matrix))
        ended = false;

    rotate_matrix(matrix);
    rotate_matrix(matrix);
    rotate_matrix(matrix);

    return ended;
}
