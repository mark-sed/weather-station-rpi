#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "pico/stdlib.h"
//#include "pico/cyw43_arch.h"
#include "wifi_config.h"
#include "Pico_ePaper_Code/c/lib/e-Paper/EPD_2in13_V2.h"
#include "Pico_ePaper_Code/c/examples/EPD_Test.h"
#include "cJSON.h"

int render_error(const char *msg) {
        UBYTE *InfoImage = malloc(
        ((EPD_2IN13_V2_WIDTH + 7) / 8) *
        EPD_2IN13_V2_HEIGHT
    );
    if (InfoImage == NULL) {
        return -1;
    }

    Paint_NewImage(
        InfoImage,
        EPD_2IN13_V2_WIDTH,
        EPD_2IN13_V2_HEIGHT,
        ROTATE_270,
        WHITE
    );

    Paint_SelectImage(InfoImage);
    Paint_SetMirroring(MIRROR_VERTICAL);
    Paint_Clear(WHITE);

    Paint_DrawLine(
        5, 22, 244, 22,
        BLACK,
        DOT_PIXEL_1X1,
        LINE_STYLE_SOLID
    );

    Paint_DrawString_EN(
        5, 39, "CHYBA!",
        &Font24,
        BLACK,
        WHITE
    );

    Paint_DrawString_EN(
        5, 86, msg,
        &Font16,
        WHITE,
        BLACK
    );

    // Additional readings
    Paint_DrawLine(
        5, 82, 244, 82,
        BLACK,
        DOT_PIXEL_1X1,
        LINE_STYLE_SOLID
    );

    // Send the completed image to the display.
    EPD_2IN13_V2_Display(InfoImage);

    free(InfoImage);

    return 0;
}

int render_info(const char *date, const char *time,
                 float outside_c, float inside_c, float feels_like_c,
                 float wind_kmh, const char *precipitation, bool warn) {
        UBYTE *InfoImage = malloc(
        ((EPD_2IN13_V2_WIDTH + 7) / 8) *
        EPD_2IN13_V2_HEIGHT
    );
    if (InfoImage == NULL) {
        return -1;
    }

    Paint_NewImage(
        InfoImage,
        EPD_2IN13_V2_WIDTH,
        EPD_2IN13_V2_HEIGHT,
        ROTATE_270,
        WHITE
    );

    Paint_SelectImage(InfoImage);
    Paint_SetMirroring(MIRROR_VERTICAL);
    Paint_Clear(WHITE);

    // Header
    Paint_DrawString_EN(
        5, 5, date,
        &Font12,
        WHITE,
        BLACK
    );

    Paint_DrawString_EN(
        200, 5, time,
        &Font12,
        WHITE,
        BLACK
    );

    Paint_DrawLine(
        5, 22, 244, 22,
        BLACK,
        DOT_PIXEL_1X1,
        LINE_STYLE_SOLID
    );

    // Main temperature
    Paint_DrawString_EN(
        5, 26, "Venku",
        &Font12,
        WHITE,
        BLACK
    );

    char out_temp_buffer[8];
    snprintf(out_temp_buffer, sizeof(out_temp_buffer), "%.1f C", outside_c);
    Paint_DrawString_EN(
        5, 39, out_temp_buffer,
        &Font24,
        WHITE,
        BLACK
    );

    // Main temperature
    Paint_DrawString_EN(
        132, 26, "Doma",
        &Font12,
        WHITE,
        BLACK
    );

    char in_temp_buffer[8];
    snprintf(in_temp_buffer, sizeof(in_temp_buffer), "%.1f C", inside_c);
    Paint_DrawString_EN(
        132, 42, in_temp_buffer,
        &Font16,
        WHITE,
        BLACK
    );

    // Additional readings
    Paint_DrawLine(
        5, 82, 244, 82,
        BLACK,
        DOT_PIXEL_1X1,
        LINE_STYLE_SOLID
    );

    char fl_temp_buffer[16];
    snprintf(fl_temp_buffer, sizeof(fl_temp_buffer), "%.0f C pocitove", feels_like_c);
    Paint_DrawString_EN(
        5, 64, fl_temp_buffer,
        &Font12,
        WHITE,
        BLACK
    );

    char wind_buffer[24];
    snprintf(wind_buffer, sizeof(wind_buffer), "Vitr: %.0f km/h", wind_kmh);
    Paint_DrawString_EN(
        5, 86, wind_buffer,
        &Font16,
        WHITE,
        BLACK
    );

    Paint_DrawString_EN(
        5, 102, precipitation,
        &Font16,
        warn ? BLACK : WHITE,
        warn ? WHITE : BLACK
    );

    // Send the completed image to the display.
    EPD_2IN13_V2_Display(InfoImage);

    free(InfoImage);

    return 0;
}

struct WeatherData {
    char date_time[64];
    float temperature_c;
    float feels_like_c;
    float wind_kmh;
    float precipitation; // TODO: Extract prediction
    bool invalid;
};

void parse_weather_json(const char *json_str, struct WeatherData *weather_data) {
    cJSON *json = cJSON_Parse(json_str);
    if (json == NULL) {
        printf("Error parsing JSON\n");
        return;
    }

    cJSON *current = cJSON_GetObjectItemCaseSensitive(json, "current");
    if (current == NULL) {
        printf("Error: 'current' object not found in JSON\n");
        cJSON_Delete(json);
        weather_data->invalid = true;
        return;
    }

    cJSON *time_item = cJSON_GetObjectItemCaseSensitive(current, "time");
    cJSON *temperature_item = cJSON_GetObjectItemCaseSensitive(current, "temperature_2m");
    cJSON *feels_like_item = cJSON_GetObjectItemCaseSensitive(current, "apparent_temperature");
    cJSON *wind_item = cJSON_GetObjectItemCaseSensitive(current, "wind_speed_10m");
    cJSON *precipitation_item = cJSON_GetObjectItemCaseSensitive(current, "precipitation");

    if (cJSON_IsString(time_item) && (time_item->valuestring != NULL)) {
        strcpy(weather_data->date_time, time_item->valuestring);
    }
    if (cJSON_IsNumber(temperature_item)) {
        weather_data->temperature_c = temperature_item->valuedouble;
    }
    if (cJSON_IsNumber(feels_like_item)) {
        weather_data->feels_like_c = feels_like_item->valuedouble;
    }
    if (cJSON_IsNumber(wind_item)) {
        weather_data->wind_kmh = wind_item->valuedouble;
    }
    if (cJSON_IsNumber(precipitation_item)) {
        weather_data->precipitation = precipitation_item->valuedouble;
    }

    weather_data->invalid = false;
    cJSON_Delete(json);
}

int format_datetime(const char *dt, char *date, size_t date_size,
                    char *time, size_t time_size) {
    int year, month, day, hour, minute;

    if (sscanf(dt, "%4d-%2d-%2dT%2d:%2d",
               &year, &month, &day, &hour, &minute) != 5)
        return -1;

    struct tm t = {0};
    t.tm_year = year - 1900;
    t.tm_mon  = month - 1;
    t.tm_mday = day;

    mktime(&t);

    const char *days[] = {
        "Ne", "Po", "Ut", "St", "Ct", "Pa", "So"
    };

    if (snprintf(date, date_size, "%s %02d.%02d.%04d",
                 days[t.tm_wday], day, month, year) >= (int)date_size)
        return -1;

    if (snprintf(time, time_size, "%02d:%02d",
                 hour, minute) >= (int)time_size)
        return -1;

    return 0;
}

int main(void) {
    stdio_init_all();
    sleep_ms(2000);

    //printf("Initializing Wi-Fi...\n");
    //if (cyw43_arch_init_with_country(CYW43_COUNTRY_CZECH_REPUBLIC) != 0) {
    //    printf("CYW43 initialization failed\n");
    //    return -1;
    //}
//
    //cyw43_arch_enable_sta_mode();
    //printf("Connecting to Wi-Fi...\n");
    //int wifi_status = cyw43_arch_wifi_connect_timeout_ms(
    //        WIFI_SSID,
    //        WIFI_PASSWORD,
    //        CYW43_AUTH_WPA2_AES_PSK,
    //        30000);
    //if (wifi_status != 0) {
    //    printf("Wi-Fi connection failed (error %d)\n", wifi_status);
    //    cyw43_arch_deinit();
    //    return -1;
    //}
    //printf("Wi-Fi connected successfully (status %d)\n", wifi_status);

    if (DEV_Module_Init() != 0) {
        return -1;
    }

    // Initialize the display for a full refresh.
    EPD_2IN13_V2_Init(EPD_2IN13_V2_FULL);
    EPD_2IN13_V2_Clear();

    const char *json_str = "{ \"latitude\": 49.206394, \"longitude\": 16.626421, \"generationtime_ms\": 0.35512447357177734, \"utc_offset_seconds\": 3600, \"timezone\": \"Europe/Prague\", \"timezone_abbreviation\": \"GMT+1\", \"elevation\": 217, \"current_units\": { \"time\": \"iso8601\", \"interval\": \"seconds\", \"temperature_2m\": \"°C\", \"apparent_temperature\": \"°C\", \"precipitation\": \"mm\", \"wind_speed_10m\": \"km/h\", \"wind_direction_10m\": \"°\", \"wind_gusts_10m\": \"km/h\" }, \"current\": { \"time\": \"2026-10-29T17:45\", \"interval\": 900, \"temperature_2m\": 10.8, \"apparent_temperature\": 8.5, \"precipitation\": 0, \"wind_speed_10m\": 7.2, \"wind_direction_10m\": 3, \"wind_gusts_10m\": 17.3 }, \"hourly_units\": { \"time\": \"iso8601\", \"precipitation_probability\": \"%\", \"precipitation\": \"mm\" }, \"hourly\": { \"time\": [ \"2026-10-29T00:00\", \"2026-10-29T01:00\", \"2026-10-29T02:00\", \"2026-10-29T03:00\", \"2026-10-29T04:00\", \"2026-10-29T05:00\", \"2026-10-29T06:00\", \"2026-10-29T07:00\", \"2026-10-29T08:00\", \"2026-10-29T09:00\", \"2026-10-29T10:00\", \"2026-10-29T11:00\", \"2026-10-29T12:00\", \"2026-10-29T13:00\", \"2026-10-29T14:00\", \"2026-10-29T15:00\", \"2026-10-29T16:00\", \"2026-10-29T17:00\", \"2026-10-29T18:00\", \"2026-10-29T19:00\", \"2026-10-29T20:00\", \"2026-10-29T21:00\", \"2026-10-29T22:00\", \"2026-10-29T23:00\", \"2026-10-30T00:00\", \"2026-10-30T01:00\", \"2026-10-30T02:00\", \"2026-10-30T03:00\", \"2026-10-30T04:00\", \"2026-10-30T05:00\", \"2026-10-30T06:00\", \"2026-10-30T07:00\", \"2026-10-30T08:00\", \"2026-10-30T09:00\", \"2026-10-30T10:00\", \"2026-10-30T11:00\", \"2026-10-30T12:00\", \"2026-10-30T13:00\", \"2026-10-30T14:00\", \"2026-10-30T15:00\", \"2026-10-30T16:00\", \"2026-10-30T17:00\", \"2026-10-30T18:00\", \"2026-10-30T19:00\", \"2026-10-30T20:00\", \"2026-10-30T21:00\", \"2026-10-30T22:00\", \"2026-10-30T23:00\" ], \"precipitation_probability\": [5,13,40,58,65,63,48,28,0,0,8,10,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,3,0 ], \"precipitation\": [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 ] } }";
    struct WeatherData weather_data;
    parse_weather_json(json_str, &weather_data);

    char ex_date[64];
    char ex_time[64];
    if (format_datetime(weather_data.date_time, ex_date, sizeof(ex_date), ex_time, sizeof(ex_time))) {
        printf("Error formatting date and time\n");
        strcpy(ex_date, "??.??.????");
        strcpy(ex_time, "??:??");
    }
    //printf("Date: %s, Time: %s\n", ex_date, ex_time);

    if (weather_data.invalid) {
        printf("Invalid weather data\n");
        if (render_error("Chyba pri ziskavani dat o pocasi")) {
            printf("Failed to render error message\n");
            return -1;
        }
    } else {
        char percip_buffer[24];
        snprintf(percip_buffer, sizeof(percip_buffer), "Dest: %.0f mm", weather_data.precipitation);
        if (render_info(ex_date, ex_time, weather_data.temperature_c, 21.45, weather_data.feels_like_c, weather_data.wind_kmh, percip_buffer, weather_data.precipitation > 0)) {
            printf("Render has failed\n");
            return -1;
        }
    }

    // Leave the image visible while the display sleeps.
    EPD_2IN13_V2_Sleep();

    DEV_Module_Exit();

    printf("Finished!\n");

    return 0;
}