#include "servo.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_console.h"
#include "esp_log.h"

// Глобальная переменная для хранения текущего угла
int current_angle = 0;

void init_servo(void)
{
    ledc_timer_config_t timer_conf = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .timer_num = LEDC_TIMER_0,
        .duty_resolution = LEDC_TIMER_13_BIT, // 13 бит для плавности
        .freq_hz = SERVO_FREQ_HZ,
        .clk_cfg = LEDC_AUTO_CLK,
    };
    ESP_ERROR_CHECK(ledc_timer_config(&timer_conf));

    ledc_channel_config_t channel_conf = {
        .gpio_num = SERVO_GPIO,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LEDC_CHANNEL_0,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = LEDC_TIMER_0,
        .duty = 0,
        .hpoint = 0,
    };
    ESP_ERROR_CHECK(ledc_channel_config(&channel_conf));
}

void set_servo_angle(int angle)
{
    // Ограничим угол
    if (angle < 0) angle = 0;
    if (angle > 180) angle = 180;

    uint32_t duty = SERVO_MIN_DUTY + ((SERVO_MAX_DUTY - SERVO_MIN_DUTY) * angle) / 180;

    // Учитываем разрешение таймера
    uint32_t max_duty = (1 << 13) - 1; // 13 бит = 8191
    uint32_t scaled_duty = (duty * max_duty) / 20000; // период в микросекундах (20ms)

    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, scaled_duty);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);

    current_angle = angle;
}

void move_servo_smooth_to(int end_angle, int step_delay_ms)
{
    if (end_angle < 0) end_angle = 0;
    if (end_angle > 180) end_angle = 180;

    int step = (current_angle < end_angle) ? 1 : -1;

    for (int angle = current_angle; angle != end_angle; angle += step) {
        set_servo_angle(angle);
        vTaskDelay(pdMS_TO_TICKS(step_delay_ms));
    }

    set_servo_angle(end_angle); // точно доехали
}

// Структуры для аргументов команд
static struct {
    struct arg_int *angle;
    struct arg_end *end;
} servo_args;

static struct {
    struct arg_int *angle;
    struct arg_int *step_delay;
    struct arg_end *end;
} servo_smooth_args;

// Обработчики команд
static int servo_cmd(int argc, char **argv)
{
    int nerrors = arg_parse(argc, argv, (void **)&servo_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, servo_args.end, argv[0]);
        return 1;
    }

    int angle = servo_args.angle->ival[0];
    printf("Setting servo to %d degrees\n", angle);
    set_servo_angle(angle);
    return 0;
}

static int servo_smooth_cmd(int argc, char **argv)
{
    int nerrors = arg_parse(argc, argv, (void **)&servo_smooth_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, servo_smooth_args.end, argv[0]);
        return 1;
    }

    int target_angle = servo_smooth_args.angle->ival[0];
    int step_delay = servo_smooth_args.step_delay->ival[0];
    printf("Moving servo smoothly from %d° to %d° with step delay %d ms\n", current_angle, target_angle, step_delay);
    move_servo_smooth_to(target_angle, step_delay);
    return 0;
}

// Регистрация команд
void register_servo_command(void)
{
    servo_args.angle = arg_int1(NULL, NULL, "<angle>", "Angle 0-180 degrees");
    servo_args.end = arg_end(1);

    const esp_console_cmd_t cmd = {
        .command = "servo",
        .help = "Set servo angle (0-180)",
        .hint = NULL,
        .func = &servo_cmd,
        .argtable = &servo_args,
    };
    ESP_ERROR_CHECK(esp_console_cmd_register(&cmd));
}

void register_servo_smooth_command(void)
{
    servo_smooth_args.angle = arg_int1(NULL, NULL, "<angle>", "Target angle 0-180 degrees");
    servo_smooth_args.step_delay = arg_int1(NULL, NULL, "<step_delay>", "Step delay in ms");
    servo_smooth_args.end = arg_end(1);

    const esp_console_cmd_t cmd = {
        .command = "servo_smooth",
        .help = "Move servo smoothly to target angle (0-180)",
        .hint = NULL,
        .func = &servo_smooth_cmd,
        .argtable = &servo_smooth_args,
    };
    ESP_ERROR_CHECK(esp_console_cmd_register(&cmd));
} 