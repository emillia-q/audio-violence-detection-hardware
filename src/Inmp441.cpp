#include "Inmp441.h"

Inmp441::Inmp441(int ws, int sd, int sck, i2s_port_t port)
{
    mic_ws = ws;
    mic_sd = sd;
    mic_sck = sck;
    i2sPort = port;
}

bool Inmp441::begin()
{
    i2s_config_t cfg = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
        .sample_rate = 16000,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
        .communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_STAND_I2S),
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,   // Just a safe practice
        .dma_buf_count = 8,
        .dma_buf_len = 64,
        .use_apll = false,
        .tx_desc_auto_clear = false,
        .fixed_mclk = 0
    };

    i2s_pin_config_t pin_cfg = {
        .bck_io_num = mic_sck,
        .ws_io_num = mic_ws,
        .data_out_num = I2S_PIN_NO_CHANGE,
        .data_in_num = mic_sd
    };

    if (i2s_driver_install(i2sPort, &cfg, 0, NULL) != ESP_OK || i2s_set_pin(i2sPort, &pin_cfg) != ESP_OK) 
        return false;
    i2s_zero_dma_buffer(i2sPort);
    return true;
}

int Inmp441::readRawData(int32_t *buffer, size_t maxSamples)
{
    size_t bytes_read = 0;
    i2s_read(i2sPort, buffer, maxSamples * sizeof(int32_t), &bytes_read, portMAX_DELAY);
    return bytes_read / sizeof(int32_t);
}
