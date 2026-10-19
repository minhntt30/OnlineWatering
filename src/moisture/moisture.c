#include "moisture.h"
#include <stddef.h>
#include "driver/gpio.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "esp_rom_sys.h"
#include "esp_log.h"

// --- Sensor analog output (AO) -> GPIO34 (ADC1_CH6, input only) ---
// Use ADC1 pins only (GPIO32..39): ADC2 cannot be used while Wi-Fi is running.
// Sensor wiring: VCC -> 3V3, GND -> GND, AO -> GPIO34
#define MOISTURE_GPIO         GPIO_NUM_34
#define MOISTURE_ATTEN        ADC_ATTEN_DB_12     // measures roughly 0..3.1 V
#define MOISTURE_SAMPLES      20                  // averaged to remove electrical noise
#define MOISTURE_SAMPLE_GAP_US 200
#define ADC_MAX_RAW           4095
#define ADC_FALLBACK_FULL_MV  3100                // used only if calibration is unavailable

static const char *TAG = "MOISTURE";

static adc_oneshot_unit_handle_t s_adc = NULL;
static adc_cali_handle_t s_cali = NULL;
static adc_channel_t s_channel;

esp_err_t moisture_init(void)
{
    adc_unit_t unit;
    esp_err_t err = adc_oneshot_io_to_channel(MOISTURE_GPIO, &unit, &s_channel);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "GPIO %d is not an ADC pin", MOISTURE_GPIO);
        return err;
    }

    const adc_oneshot_unit_init_cfg_t unit_cfg = {
        .unit_id = unit,
    };
    err = adc_oneshot_new_unit(&unit_cfg, &s_adc);
    if (err != ESP_OK) {
        return err;
    }

    const adc_oneshot_chan_cfg_t chan_cfg = {
        .atten    = MOISTURE_ATTEN,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
    err = adc_oneshot_config_channel(s_adc, s_channel, &chan_cfg);
    if (err != ESP_OK) {
        return err;
    }

#if ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED
    const adc_cali_line_fitting_config_t cali_cfg = {
        .unit_id  = unit,
        .atten    = MOISTURE_ATTEN,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
    if (adc_cali_create_scheme_line_fitting(&cali_cfg, &s_cali) != ESP_OK) {
        s_cali = NULL;
    }
#endif
    if (s_cali == NULL) {
        ESP_LOGW(TAG, "ADC calibration unavailable, voltage will be approximate");
    }

    ESP_LOGI(TAG, "Moisture sensor ready on GPIO %d", MOISTURE_GPIO);
    return ESP_OK;
}

int moisture_read_raw(void)
{
    if (s_adc == NULL) {
        return -1;
    }

    int sum = 0;
    for (int i = 0; i < MOISTURE_SAMPLES; i++) {
        int raw;
        if (adc_oneshot_read(s_adc, s_channel, &raw) != ESP_OK) {
            return -1;
        }
        sum += raw;
        esp_rom_delay_us(MOISTURE_SAMPLE_GAP_US);
    }
    return sum / MOISTURE_SAMPLES;
}

float moisture_read_voltage(void)
{
    int raw = moisture_read_raw();
    if (raw < 0) {
        return -1.0f;
    }

    int mv;
    if (s_cali != NULL && adc_cali_raw_to_voltage(s_cali, raw, &mv) == ESP_OK) {
        return mv / 1000.0f;
    }
    return (raw * ADC_FALLBACK_FULL_MV / ADC_MAX_RAW) / 1000.0f;
}