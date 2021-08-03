#include "esp_http_server.h"
#include "esp_timer.h"
#include "esp_camera.h"
#include "img_converters.h"
#include "camera_index.h"
#include "Arduino.h"

#include "fb_gfx.h"
#include "fd_forward.h"
#include "fr_forward.h"


typedef struct {
        size_t size; //number of values used for filtering
        size_t index; //current value index
        size_t count; //value count
        int sum;
        int * values; //array to be filled with values
} ra_filter_t;

typedef struct {
        httpd_req_t *req;
        size_t len;
} jpg_chunking_t;

#define PART_BOUNDARY "123456789000000000000987654321"
static const char* _STREAM_CONTENT_TYPE = "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
static const char* _STREAM_BOUNDARY = "\r\n--" PART_BOUNDARY "\r\n";
static const char* _STREAM_PART = "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n";

//static ra_filter_t ra_filter;
//httpd_handle_t stream_httpd = NULL;
//httpd_handle_t camera_httpd = NULL;

//static mtmn_config_t mtmn_config = {0};
//static int8_t detection_enabled = 0;
//static int8_t recognition_enabled = 0;
//static int8_t is_enrolling = 0;
//static face_id_list id_list = {0};
//
//static ra_filter_t * ra_filter_init(ra_filter_t * filter, size_t sample_size){
//    memset(filter, 0, sizeof(ra_filter_t));
//
//    filter->values = (int *)malloc(sample_size * sizeof(int));
//    if(!filter->values){
//        return NULL;
//    }
//    memset(filter->values, 0, sample_size * sizeof(int));
//
//    filter->size = sample_size;
//    return filter;
//}
//
//static int ra_filter_run(ra_filter_t * filter, int value){
//    if(!filter->values){
//        return value;
//    }
//    filter->sum -= filter->values[filter->index];
//    filter->values[filter->index] = value;
//    filter->sum += filter->values[filter->index];
//    filter->index++;
//    filter->index = filter->index % filter->size;
//    if (filter->count < filter->size) {
//        filter->count++;
//    }
//    return filter->sum / filter->count;
//}
//
//static void rgb_print(dl_matrix3du_t *image_matrix, uint32_t color, const char * str){
//    fb_data_t fb;
//    fb.width = image_matrix->w;
//    fb.height = image_matrix->h;
//    fb.data = image_matrix->item;
//    fb.bytes_per_pixel = 3;
//    fb.format = FB_BGR888;
//    fb_gfx_print(&fb, (fb.width - (strlen(str) * 14)) / 2, 10, color, str);
//}
//
//static int rgb_printf(dl_matrix3du_t *image_matrix, uint32_t color, const char *format, ...){
//    char loc_buf[64];
//    char * temp = loc_buf;
//    int len;
//    va_list arg;
//    va_list copy;
//    va_start(arg, format);
//    va_copy(copy, arg);
//    len = vsnprintf(loc_buf, sizeof(loc_buf), format, arg);
//    va_end(copy);
//    if(len >= sizeof(loc_buf)){
//        temp = (char*)malloc(len+1);
//        if(temp == NULL) {
//            return 0;
//        }
//    }
//    vsnprintf(temp, len+1, format, arg);
//    va_end(arg);
//    rgb_print(image_matrix, color, temp);
//    if(len > 64){
//        free(temp);
//    }
//    return len;
//}

extern int FTP_uploadImage(int64_t t, unsigned char * pdata, unsigned int size, String namefile);
int end_name = 1;
int capture_ftpupload(void)
{
    String nama = String(end_name);
  	camera_fb_t * fb = NULL;
    esp_err_t res = ESP_OK;
    int64_t fr_start = esp_timer_get_time();

    fb = esp_camera_fb_get();
    if (!fb) {
        Serial.println("Camera capture failed");

        return ESP_FAIL;
    }
    

    FTP_uploadImage(fr_start, fb->buf, fb->len, nama);

	  esp_camera_fb_return(fb);
    end_name += 1;

    return res;
}
