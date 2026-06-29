#pragma once

#include <Arduino.h>
#include <driver/i2s.h>

class Inmp441 {
    int mic_ws;
    int mic_sd;
    int mic_sck;
    i2s_port_t i2sPort;

public: 
    Inmp441(int ws, int sd, int sck, i2s_port_t port = I2S_NUM_0);

    bool begin();
    int readSamples(int16_t* buffer, size_t maxSamples);
};