// Generic Arduino
#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <lvgl.h>
#include <lv_tc.h>
#include <lv_tc_screen.h>
#include <esp_nvs_tc.h>
#include <vector.h>
#include <EEPROM.h>
#include "tft_test.h"
#include "lv_demo_stress.h"

#define EEPROM_SIZE 128
#define EEPROM_ADDR_WIFI_FLAG 0
#define EEPROM_ADDR_WIFI_CREDENTIAL 4

// Instantiate display
TFT_eSPI tft = TFT_eSPI();
bool factoryReset = 0;

void do_lvgl_init();
void loadWIFICredentialEEPROM();
void saveWIFICredentialEEPROM(int, String );
// lvgl stuff
// some main screen defines
static const uint16_t screenWidth  = 320;
static const uint16_t screenHeight = 240;
static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[ screenWidth * screenHeight / 10 ];
static lv_style_t label_style;

// some constants for various display elements
#define MS_HIEGHT (tft.height() - 37) // allow for a little room at the sides
#define MS_WIDTH (tft.width() - 2) 
#define STATUS_BAR_HIEGHT 30

/* lvgl Display flushing */
void my_disp_flush( lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p )
{
    uint32_t w = ( area->x2 - area->x1 + 1 );
    uint32_t h = ( area->y2 - area->y1 + 1 );

    tft.startWrite();
    tft.setAddrWindow( area->x1, area->y1, w, h );
    tft.pushColors( ( uint16_t * )&color_p->full, w * h, true );
    tft.endWrite();

    lv_disp_flush_ready( disp_drv );
} // end my_disp_flush

/* Read the touchpad */
void my_touchpad_read( lv_indev_drv_t * indev_drv, lv_indev_data_t * data )
{
    uint16_t touchX, touchY;

    bool touched = tft.getTouch( &touchX, &touchY, 600 );

    if( !touched )
    {
        data->state = LV_INDEV_STATE_REL;
    }
    else
    {
        data->state = LV_INDEV_STATE_PR;

        /*Set the coordinates*/
        data->point.x = touchX;
        data->point.y = touchY;
    }
} // end my_touchpad_read

// used by tc calibrate, just restart after tc is calibrated
void your_tc_finish_cb(
        lv_event_t *event
    ) {
    // Calibration finished so run stress test
    // which cleans up the cal screen etc.
    lv_demo_stress();

} // end your_tc_finish_cb


// LVGL Basic Example Code
// *****Example 1
//#if 0
static void event_cb(lv_event_t * e)
{
    LV_LOG_USER("Clicked");

    static uint32_t cnt = 1;
    lv_obj_t * btn = lv_event_get_target(e);
    lv_obj_t * label = lv_obj_get_child(btn, 0);
    lv_label_set_text_fmt(label, "%" LV_PRIu32, cnt);
    cnt++;
}

/**
 * Add click event to a button
 */
void lv_example_event_1(void)
{
    lv_obj_t * btn = lv_btn_create(lv_scr_act());
    lv_obj_set_size(btn, 100, 50);
    lv_obj_center(btn);
    lv_obj_add_event_cb(btn, event_cb, LV_EVENT_PRESSED, NULL);

    lv_obj_t * label = lv_label_create(btn);
    lv_label_set_text(label, "Click me!");
    lv_obj_center(label);
}
//#endif

// init lvgl graphics
void do_lvgl_init(){

    String LVGL_Arduino = "LVGL: ";
    LVGL_Arduino += String('V') + lv_version_major() + "." + lv_version_minor() + "." + lv_version_patch();
    Serial.println( LVGL_Arduino );
    
    // init the screen hw
    tft.begin();          /* TFT init */
    tft.setRotation( 1 ); /* 3 = landscape/flipped 1 = landscape/no flip*/
    // init graphics lib
    lv_init();
    lv_disp_draw_buf_init( &draw_buf, buf, NULL, screenWidth * screenHeight / 10 );

    /*Initialize the display*/
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init( &disp_drv );
    /*Change the following line to your display resolution*/
    disp_drv.hor_res = screenWidth;
    disp_drv.ver_res = screenHeight;
    disp_drv.flush_cb = my_disp_flush;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register( &disp_drv );

    /*Initialize the (dummy) input device driver*/
    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init( &indev_drv );
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = my_touchpad_read;
    
    lv_tc_indev_drv_init(&indev_drv, my_touchpad_read);
    lv_indev_drv_register( &indev_drv );

    // Calibration code
    /*
        If using NVS:
        Register a calibration coefficients save callback.
    */
   
   #if 0
   // this will force a recalibration by erasing the nvs data
   #if defined (ERASE_TC_CAL)
        // always erase cal data - remove later
    esp_nvs_tc_coeff_erase();
   #endif
    #endif
    // register function to save cal data
    lv_tc_register_coeff_save_cb(esp_nvs_tc_coeff_save_cb);

    // if cal data exists then we are good to go, otherwise cal the calibration routine 
    if(esp_nvs_tc_coeff_init()) {
        //run screen stress test code
        lv_demo_stress();
    } else {
        /*
            There is no cal data: load the calibration screen, perform the calibration, then restart
        */
        // tc cal screen
        lv_obj_t *tCScreen = lv_tc_screen_create();
        // call back for tc cal finished
        lv_obj_add_event_cb(tCScreen, your_tc_finish_cb, LV_EVENT_READY, NULL);

        // do the tc cal, when complete we get the LV_EVENT_READY event for the tCScreen object
        lv_disp_load_scr(tCScreen);
        lv_tc_screen_start(tCScreen);
    } // end if
} // end do_lvgl_init


// test the tft connections
void tft_test(int tsCal){
    
    // erase any stored touch calibration
    if (tsCal != 0)
      // Force cal
      esp_nvs_tc_coeff_erase();
    
    // initialize the tft
    do_lvgl_init();

    // now loop and poll the graphics handler
    while(1) {
        lv_timer_handler(); /* let the GUI do its work */
    
        // check for key pess
        if(Serial.read() != -1){
          // clean up
          lv_demo_stress_close();
          return;
        }

        // little delay
        delay(10);

    } // end while
} // end tft_test
