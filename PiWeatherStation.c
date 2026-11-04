#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "config.h"
#include "lwip/altcp_tls.h"
#include "lwip/dns.h"
#include "lwip/prot/iana.h"
#include "mbedtls/debug.h"
#include "mbedtls/error.h"
#include "mbedtls/platform_time.h"
#include "mbedtls/ssl.h"
#include "mbedtls/version.h"
#include "Pico_ePaper_Code/c/lib/e-Paper/EPD_2in13_V2.h"
#include "Pico_ePaper_Code/c/examples/EPD_Test.h"
#include "cJSON.h"

#define DISPLAY_IMAGE_SIZE (((EPD_2IN13_V2_WIDTH + 7) / 8) * EPD_2IN13_V2_HEIGHT)

static UBYTE displayed_image[DISPLAY_IMAGE_SIZE];
static bool displayed_image_valid;

static void remember_display_image(const UBYTE *image) {
    memcpy(displayed_image, image, sizeof(displayed_image));
    displayed_image_valid = true;
}

mbedtls_ms_time_t mbedtls_ms_time(void) {
    return (mbedtls_ms_time_t)(time_us_64() / 1000);
}

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
    remember_display_image(InfoImage);

    free(InfoImage);

    return 0;
}

int render_message(const char *msg) {
        UBYTE *InfoImage = malloc(
        ((EPD_2IN13_V2_WIDTH + 7) / 8) *
        EPD_2IN13_V2_HEIGHT
    );
    if (InfoImage == NULL) {
        return -1;
    }

    if (displayed_image_valid) {
        memcpy(InfoImage, displayed_image, sizeof(displayed_image));
    } else {
        memset(InfoImage, WHITE, sizeof(displayed_image));
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

    Paint_ClearWindows(0, 81, 250, 122, BLACK);

    Paint_DrawString_EN(
        5, 86, msg,
        &Font16,
        BLACK,
        WHITE
    );

    // Send the completed image to the display.
    EPD_2IN13_V2_Display(InfoImage);
    remember_display_image(InfoImage);

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
        132, 42, "??.?",
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

    char fl_temp_buffer[8];
    snprintf(fl_temp_buffer, sizeof(fl_temp_buffer), "%.0f C", feels_like_c);
    Paint_DrawString_EN(
        5, 64, fl_temp_buffer,
        &Font16,
        WHITE,
        BLACK
    );
    Paint_DrawString_EN(
        48, 66, "pocitove",
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
    remember_display_image(InfoImage);

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

#define WEATHER_API_HOST "api.open-meteo.com"
#define WEATHER_RESPONSE_CAPACITY 12288

#define UPDATE_FREQ 1800000 // 30 minutes

struct WeatherHttpState {
    volatile bool dns_complete;
    volatile bool request_complete;
    volatile bool failed;
    volatile bool connected;
    ip_addr_t address;
    struct altcp_tls_config *tls_config;
    struct altcp_pcb *pcb;
    size_t response_length;
    const char *failure_reason;
    err_t failure_code;
    char request[640];
    char response[WEATHER_RESPONSE_CAPACITY];
};

static struct WeatherHttpState weather_http_state;

static void weather_dns_callback(const char *name, const ip_addr_t *resolved,
                                 void *arg) {
    (void)name;
    struct WeatherHttpState *state = arg;
    if (resolved == NULL) {
        state->failed = true;
        state->failure_reason = "DNS lookup";
    } else {
        state->address = *resolved;
    }
    state->dns_complete = true;
}

static void weather_error_callback(void *arg, err_t error) {
    printf("TLS error callback: err=%d\n", error);
    struct WeatherHttpState *state = arg;
    if (error == ERR_CLSD && state->response_length > 0) {
        state->request_complete = true;
        state->pcb = NULL;
        return;
    }
    state->failed = true;
    state->failure_reason = state->connected
        ? "connection closed after TLS connect"
        : "connection closed during TLS handshake";
    state->failure_code = error;
    state->request_complete = true;
    state->pcb = NULL;
}

static err_t weather_connected_callback(void *arg, struct altcp_pcb *pcb,
                                        err_t error) {
    printf("TLS connected callback: err=%d\n", error);
    struct WeatherHttpState *state = arg;
    if (error != ERR_OK) {
        state->failed = true;
        state->failure_reason = "TCP/TLS connect callback";
        state->failure_code = error;
        state->request_complete = true;
        return ERR_OK;
    }
    state->connected = true;

    err_t write_error = altcp_write(
        pcb,
        state->request,
        (u16_t)strlen(state->request),
        TCP_WRITE_FLAG_COPY
    );
    if (write_error == ERR_OK) {
        write_error = altcp_output(pcb);
    }
    if (write_error != ERR_OK) {
        state->failed = true;
        state->failure_reason = "HTTP request write";
        state->failure_code = write_error;
        state->request_complete = true;
        altcp_abort(pcb);
        state->pcb = NULL;
        return ERR_ABRT;
    }
    return ERR_OK;
}

static err_t weather_receive_callback(void *arg, struct altcp_pcb *pcb,
                                      struct pbuf *packet, err_t error) {
    struct WeatherHttpState *state = arg;
    if (error != ERR_OK) {
        state->failed = true;
        state->failure_reason = "HTTP response receive";
        state->failure_code = error;
        state->request_complete = true;
        if (packet != NULL) {
            pbuf_free(packet);
        }
        return ERR_OK;
    }
    if (packet == NULL) {
        state->request_complete = true;
        return ERR_OK;
    }

    for (struct pbuf *part = packet; part != NULL; part = part->next) {
        if (part->len >= WEATHER_RESPONSE_CAPACITY - state->response_length) {
            state->failed = true;
            state->failure_reason = "HTTP response exceeds buffer";
            state->request_complete = true;
            pbuf_free(packet);
            altcp_abort(pcb);
            state->pcb = NULL;
            return ERR_ABRT;
        }
        memcpy(state->response + state->response_length, part->payload, part->len);
        state->response_length += part->len;
    }
    state->response[state->response_length] = '\0';
    altcp_recved(pcb, packet->tot_len);
    pbuf_free(packet);
    return ERR_OK;
}

static bool weather_wait_for(volatile bool *flag) {
    uint64_t deadline = time_us_64() + 20000000;
    while (!*flag && time_us_64() < deadline) {
        sleep_ms(10);
    }
    return *flag;
}

static bool weather_extract_json(void) {
    char *header_end = strstr(weather_http_state.response, "\r\n\r\n");
    if (header_end == NULL ||
        (strncmp(weather_http_state.response, "HTTP/1.1 200", 12) != 0 &&
         strncmp(weather_http_state.response, "HTTP/1.0 200", 12) != 0)) {
        return false;
    }

    char *body = header_end + 4;
    size_t body_length = weather_http_state.response_length -
                         (size_t)(body - weather_http_state.response);
    if (strstr(weather_http_state.response, "Transfer-Encoding: chunked") != NULL) {
        char *read_cursor = body;
        char *write_cursor = weather_http_state.response;
        char *response_end = weather_http_state.response + weather_http_state.response_length;
        while (read_cursor < response_end) {
            char *line_end = strstr(read_cursor, "\r\n");
            char *size_end;
            if (line_end == NULL || line_end >= response_end) {
                return false;
            }
            unsigned long chunk_size = strtoul(read_cursor, &size_end, 16);
            if (size_end == read_cursor || size_end > line_end) {
                return false;
            }
            read_cursor = line_end + 2;
            if (chunk_size == 0) {
                *write_cursor = '\0';
                weather_http_state.response_length = (size_t)(write_cursor - weather_http_state.response);
                return true;
            }
            if ((size_t)(response_end - read_cursor) < chunk_size + 2 ||
                read_cursor[chunk_size] != '\r' || read_cursor[chunk_size + 1] != '\n') {
                return false;
            }
            memmove(write_cursor, read_cursor, chunk_size);
            write_cursor += chunk_size;
            read_cursor += chunk_size + 2;
        }
        return false;
    }

    memmove(weather_http_state.response, body, body_length);
    weather_http_state.response[body_length] = '\0';
    weather_http_state.response_length = body_length;
    return body_length != 0;
}

static void mbedtls_debug_callback(
    void *ctx,
    int level,
    const char *file,
    int line,
    const char *str
) {
    printf("TLS[%d] %s:%d: %s", level, file, line, str);
}

static const char *fetch_weather_json(void) {
    struct WeatherHttpState *state = &weather_http_state;
    memset(state, 0, sizeof(*state));
    mbedtls_debug_set_threshold(4);

    int request_length = snprintf(
        state->request,
        sizeof(state->request),
        "GET /v1/forecast?latitude=%.6f&longitude=%.6f&current=temperature_2m,apparent_temperature,precipitation,wind_speed_10m,wind_direction_10m,wind_gusts_10m&hourly=precipitation_probability,precipitation&forecast_days=2&timezone=Europe%%2FPrague HTTP/1.1\r\n"
        "Host: " WEATHER_API_HOST "\r\n"
        "Accept: application/json\r\n"
        "Accept-Encoding: identity\r\n"
        "Connection: close\r\n\r\n",
        (double)LATITUDE,
        (double)LONGITUDE
    );
    if (request_length < 0 || (size_t)request_length >= sizeof(state->request)) {
        state->failure_reason = "request URL formatting";
        return NULL;
    }

    cyw43_arch_lwip_begin();
    err_t dns_error = dns_gethostbyname(
        WEATHER_API_HOST,
        &state->address,
        weather_dns_callback,
        state
    );
    cyw43_arch_lwip_end();
    if (dns_error == ERR_OK) {
        state->dns_complete = true;
    } else if (dns_error != ERR_INPROGRESS) {
        state->failed = true;
        state->failure_reason = "DNS query start";
        state->failure_code = dns_error;
    }
    if (state->failed || !weather_wait_for(&state->dns_complete)) {
        state->failed = true;
        if (state->failure_reason == NULL) {
            state->failure_reason = "DNS timeout";
        }
        return NULL;
    }

    cyw43_arch_lwip_begin();
    state->tls_config = altcp_tls_create_config_client(NULL, 0);
    if (state->tls_config == NULL) {
        printf("Failed to create TLS config\n");
        return NULL;
    }
    if (state->tls_config != NULL) {
        state->pcb = altcp_tls_new(state->tls_config, IPADDR_TYPE_ANY);
    }
    cyw43_arch_lwip_end();
    if (state->pcb == NULL) {
        state->failed = true;
        state->failure_reason = "TLS allocation";
    } else {
        mbedtls_ssl_context *ssl = altcp_tls_context(state->pcb);
        if (ssl == NULL || mbedtls_ssl_set_hostname(ssl, WEATHER_API_HOST) != 0) {
            state->failed = true;
            state->failure_reason = "TLS SNI setup";
        }
    }

    if (!state->failed) {
        cyw43_arch_lwip_begin();
        altcp_arg(state->pcb, state);
        altcp_err(state->pcb, weather_error_callback);
        altcp_recv(state->pcb, weather_receive_callback);
        err_t connect_error = altcp_connect(
            state->pcb,
            &state->address,
            LWIP_IANA_PORT_HTTPS,
            weather_connected_callback
        );
        cyw43_arch_lwip_end();
        if (connect_error != ERR_OK) {
            state->failed = true;
            state->failure_reason = "TCP/TLS connect start";
            state->failure_code = connect_error;
        } else if (!weather_wait_for(&state->request_complete)) {
            state->failed = true;
            state->failure_reason = "TCP/TLS or HTTP response timeout";
        }
    }

    if (state->pcb != NULL) {
        cyw43_arch_lwip_begin();
        if (state->failed || altcp_close(state->pcb) != ERR_OK) {
            altcp_abort(state->pcb);
        }
        state->pcb = NULL;
        cyw43_arch_lwip_end();
    }
    if (state->tls_config != NULL) {
        cyw43_arch_lwip_begin();
        altcp_tls_free_config(state->tls_config);
        state->tls_config = NULL;
        cyw43_arch_lwip_end();
    }

    if (state->failed || !weather_extract_json()) {
        if (state->failure_reason == NULL) {
            state->failure_reason = "HTTP status or body parsing";
        }
        printf("Weather API failure: %s (%d)\n",
               state->failure_reason,
               state->failure_code);
         printf("Weather response bytes received: %u\n",
             (unsigned int)state->response_length);
        return NULL;
    }
    return state->response;
}

void parse_weather_json(const char *json_str, struct WeatherData *weather_data) {
    weather_data->invalid = true;
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

    if (DEV_Module_Init() != 0) {
        return -1;
    }

    bool display_cleared = false;
    while (true) {
        // Reinitialize the panel after its sleep at the end of each cycle.
        EPD_2IN13_V2_Init(EPD_2IN13_V2_FULL);
        if (!display_cleared) {
            EPD_2IN13_V2_Clear();
            display_cleared = true;
        }

        printf("mbedTLS version: %s\n", MBEDTLS_VERSION_STRING);
        printf("Initializing Wi-Fi...\n");
        render_message("Inicializace Wi-Fi");

        if (cyw43_arch_init_with_country(CYW43_COUNTRY_CZECH_REPUBLIC) != 0) {
            printf("CYW43 initialization failed\n");
            render_error("Chyba inicializace Wi-Fi");
        } else {
            cyw43_arch_enable_sta_mode();
            printf("Connecting to Wi-Fi...\n");
            render_message("Pripojovani k Wi-Fi");
            int wifi_status = cyw43_arch_wifi_connect_timeout_ms(
                WIFI_SSID,
                WIFI_PASSWORD,
                CYW43_AUTH_WPA2_AES_PSK,
                30000
            );
            if (wifi_status != 0) {
                printf("Wi-Fi connection failed (error %d)\n", wifi_status);
                render_error("Chyba pripojeni k Wi-Fi");
            } else {
                render_message("Pripojeno k Wi-Fi");
                sleep_ms(500);
                printf("Wi-Fi connected successfully (status %d)\n", wifi_status);

                //const char *json_str = "{ \"latitude\": 49.206394, \"longitude\": 16.626421, \"generationtime_ms\": 0.35512447357177734, \"utc_offset_seconds\": 3600, \"timezone\": \"Europe/Prague\", \"timezone_abbreviation\": \"GMT+1\", \"elevation\": 217, \"current_units\": { \"time\": \"iso8601\", \"interval\": \"seconds\", \"temperature_2m\": \"°C\", \"apparent_temperature\": \"°C\", \"precipitation\": \"mm\", \"wind_speed_10m\": \"km/h\", \"wind_direction_10m\": \"°\", \"wind_gusts_10m\": \"km/h\" }, \"current\": { \"time\": \"2026-10-30T17:45\", \"interval\": 900, \"temperature_2m\": 7.8, \"apparent_temperature\": 8.5, \"precipitation\": 0, \"wind_speed_10m\": 7.2, \"wind_direction_10m\": 3, \"wind_gusts_10m\": 17.3 }, \"hourly_units\": { \"time\": \"iso8601\", \"precipitation_probability\": \"%\", \"precipitation\": \"mm\" }, \"hourly\": { \"time\": [ \"2026-10-29T00:00\", \"2026-10-29T01:00\", \"2026-10-29T02:00\", \"2026-10-29T03:00\", \"2026-10-29T04:00\", \"2026-10-29T05:00\", \"2026-10-29T06:00\", \"2026-10-29T07:00\", \"2026-10-29T08:00\", \"2026-10-29T09:00\", \"2026-10-29T10:00\", \"2026-10-29T11:00\", \"2026-10-29T12:00\", \"2026-10-29T13:00\", \"2026-10-29T14:00\", \"2026-10-29T15:00\", \"2026-10-29T16:00\", \"2026-10-29T17:00\", \"2026-10-29T18:00\", \"2026-10-29T19:00\", \"2026-10-29T20:00\", \"2026-10-29T21:00\", \"2026-10-29T22:00\", \"2026-10-29T23:00\", \"2026-10-30T00:00\", \"2026-10-30T01:00\", \"2026-10-30T02:00\", \"2026-10-30T03:00\", \"2026-10-30T04:00\", \"2026-10-30T05:00\", \"2026-10-30T06:00\", \"2026-10-30T07:00\", \"2026-10-30T08:00\", \"2026-10-30T09:00\", \"2026-10-30T10:00\", \"2026-10-30T11:00\", \"2026-10-30T12:00\", \"2026-10-30T13:00\", \"2026-10-30T14:00\", \"2026-10-30T15:00\", \"2026-10-30T16:00\", \"2026-10-30T17:00\", \"2026-10-30T18:00\", \"2026-10-30T19:00\", \"2026-10-30T20:00\", \"2026-10-30T21:00\", \"2026-10-30T22:00\", \"2026-10-30T23:00\" ], \"precipitation_probability\": [5,13,40,58,65,63,48,28,0,0,8,10,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,3,0 ], \"precipitation\": [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 ] } }";
                const char *json_str = fetch_weather_json();
                if (json_str == NULL) {
                    printf("Weather API request failed\n");
                } else {
                    //printf("Weather API response:\n%s\n", json_str);
                }

                struct WeatherData weather_data = {0};
                weather_data.invalid = true;
                if (json_str != NULL) {
                    parse_weather_json(json_str, &weather_data);
                }

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

            }
            cyw43_arch_deinit();
        }

        printf("Sleeping for %d ms...\n", UPDATE_FREQ);
        EPD_2IN13_V2_Sleep();
        sleep_ms(UPDATE_FREQ);
    }
}