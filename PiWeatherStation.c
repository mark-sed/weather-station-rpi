#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "wifi_config.h"
#include "Pico_ePaper_Code/c/lib/e-Paper/EPD_2in13_V2.h"
#include "Pico_ePaper_Code/c/examples/EPD_Test.h"

int main(void)
{
    stdio_init_all();
    sleep_ms(2000);

    printf("Initializing Wi-Fi...\n");
    if (cyw43_arch_init_with_country(CYW43_COUNTRY_CZECH_REPUBLIC) != 0) {
        printf("CYW43 initialization failed\n");
        return -1;
    }

    cyw43_arch_enable_sta_mode();
    printf("Connecting to Wi-Fi...\n");
    int wifi_status = cyw43_arch_wifi_connect_timeout_ms(
            WIFI_SSID,
            WIFI_PASSWORD,
            CYW43_AUTH_WPA2_AES_PSK,
            30000);
    if (wifi_status != 0) {
        printf("Wi-Fi connection failed (error %d)\n", wifi_status);
        cyw43_arch_deinit();
        return -1;
    }
    printf("Wi-Fi connected successfully (status %d)\n", wifi_status);

    if (DEV_Module_Init() != 0) {
        return -1;
    }

    // Initialize the display for a full refresh.
    EPD_2IN13_V2_Init(EPD_2IN13_V2_FULL);
    EPD_2IN13_V2_Clear();

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
        5, 5, "St 28.10.2026",
        &Font12,
        WHITE,
        BLACK
    );

    Paint_DrawString_EN(
        200, 5, "14:32",
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

    Paint_DrawString_EN(
        5, 39, "10.6 C",
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

    Paint_DrawString_EN(
        132, 42, "21.6 C",
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

    Paint_DrawString_EN(
        5, 64, "10.0 C pocitove",
        &Font12,
        WHITE,
        BLACK
    );

    Paint_DrawString_EN(
        5, 86, "Vitr:  3 km/h",
        &Font16,
        WHITE,
        BLACK
    );

    Paint_DrawString_EN(
        5, 102, "Dest:  0.1 mm v 16 h",
        &Font16,
        BLACK,
        WHITE // Switch colors if rain/snow is predicted today
    );

    // Send the completed image to the display.
    EPD_2IN13_V2_Display(InfoImage);

    free(InfoImage);

    // Leave the image visible while the display sleeps.
    EPD_2IN13_V2_Sleep();

    DEV_Module_Exit();

    return 0;
}