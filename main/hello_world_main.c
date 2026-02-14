#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/i2c.h"

#define SLAVE_ADDR 0x26

#define I2C_MASTER_NUM I2C_NUM_0
#define I2C_MASTER_FREQ_HZ 100000
#define I2C_MASTER_SCL_IO 22
#define I2C_MASTER_SDA_IO 21

TaskHandle_t i2cTaskHandle;
TaskHandle_t processDataTaskHandle;
TaskHandle_t backendTaskHandle;

void init_i2c(void)
{
    i2c_config_t config = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ
    };

    i2c_param_config(I2C_MASTER_NUM, &config);
    i2c_driver_install(I2C_MASTER_NUM, config.mode, 0, 0, 0);
}

void i2cTask(void* pvParameters)
{
    uint8_t temp, humi;

    while (1)
    {
        i2c_cmd_handle_t cmd = i2c_cmd_link_create();

        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, (SLAVE_ADDR << 1) | I2C_MASTER_READ, true);
        i2c_master_read_byte(cmd, &temp, I2C_MASTER_ACK);
        i2c_master_read_byte(cmd, &humi, I2C_MASTER_NACK);
        i2c_master_stop(cmd);

        i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, portMAX_DELAY);
        i2c_cmd_link_delete(cmd);

        printf("I2C Read Done\n");

        xTaskNotifyGive(processDataTaskHandle);

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void processDataTask(void* pvParameters)
{
    while (1)
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        printf("Processing temperature\n");
        printf("Processing humidity\n");

        xTaskNotifyGive(backendTaskHandle);
    }
}

void backendTask(void* pvParameters)
{
    while (1)
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        printf("Sending data to backend...\n");
    }
}

void app_main(void)
{
    init_i2c();

    xTaskCreate(i2cTask,
                "i2cTask",
                4096,
                NULL,
                5,
                &i2cTaskHandle);

    xTaskCreate(processDataTask,
                "processDataTask",
                4096,
                NULL,
                4,
                &processDataTaskHandle);

    xTaskCreate(backendTask,
                "backendTask",
                4096,
                NULL,
                3,
                &backendTaskHandle);
}
