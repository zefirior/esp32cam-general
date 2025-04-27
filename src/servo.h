#ifndef SERVO_H
#define SERVO_H

#include <driver/ledc.h>
#include "esp_err.h"
#include "argtable3/argtable3.h"

// Константы для сервопривода
#define SERVO_GPIO 15
#define SERVO_MIN_DUTY 500    // 0.5ms
#define SERVO_MAX_DUTY 2500   // 2.5ms
#define SERVO_FREQ_HZ 50      // 50Hz стандартно для сервоприводов

// Глобальная переменная для хранения текущего угла
extern int current_angle;

// Функции инициализации и управления сервоприводом
void init_servo(void);
void set_servo_angle(int angle);
void move_servo_smooth_to(int end_angle, int step_delay_ms);

// Функции для регистрации консольных команд
void register_servo_command(void);
void register_servo_smooth_command(void);

#endif // SERVO_H