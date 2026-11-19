#ifndef BUTTON_H
#define BUTTON_H



#include <stdbool.h>

/* События, которые модуль отдаёт наружу */
typedef enum {
    BUTTON_EVT_NONE = 0,
    BUTTON_EVT_PRESS,
    BUTTON_EVT_RELEASE,
} button_event_t;

/* Инициализация модуля. Возвращает 0 при успехе, <0 при ошибке. */
int button_init(void);

/*
 * Опрос кнопки. Вызывать периодически (например, из главного цикла).
 * Возвращает событие с момента прошлого вызова:
 *   BUTTON_EVT_PRESS   — был фронт нажатия
 *   BUTTON_EVT_RELEASE — был фронт отпускания
 *   BUTTON_EVT_NONE    — ничего не произошло
 * Внутри реализован антидребезг и ограничение частоты опроса.
 */
button_event_t button_poll(void);





#endif 

