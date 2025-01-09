#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_err.h"
#include "nvs_flash.h"
#include "protocol_examples_common.h"
#include "file_serving_example_common.h"


#define DYNAMIC_BUFFER_SIZE 64

char dynamic_buffer[DYNAMIC_BUFFER_SIZE];

typedef enum {
  INDEX_HTML,
  CONTENIDO_HTML,
  FAVICON_ICO,
  CONTENIDO_DINAMICO_JS,
  //Always last
  URI_COUNT
} uris_enum;



extern const unsigned char index_html_start[] asm("_binary_index_html_start");
extern const unsigned char index_html_end[] asm("_binary_index_html_end");
extern const unsigned char contenido_html_start[] asm("_binary_contenido_html_start");
extern const unsigned char contenido_html_end[] asm("_binary_contenido_html_end");
extern const unsigned char favicon_ico_start[] asm("_binary_favicon_ico_start");
extern const unsigned char favicon_ico_end[]   asm("_binary_favicon_ico_end");
extern const unsigned char contenido_dinamico_js_start[] asm("_binary_contenido_dinamico_js_start");
extern const unsigned char contenido_dinamico_js_end[]   asm("_binary_contenido_dinamico_js_end");

const wh_uri_type uri_table[URI_COUNT] = {

  [INDEX_HTML] = {
        .uri_name = "/index.html",
        .uri_start = index_html_start,
        .uri_end = index_html_end
  },
  [CONTENIDO_HTML] = {
        .uri_name = "/contenido.html",
        .uri_start = contenido_html_start,
        .uri_end = contenido_html_end
  },
 [FAVICON_ICO] = {
        .uri_name = "/favicon.ico",
        .uri_start = favicon_ico_start,
        .uri_end = favicon_ico_end
  },
  [CONTENIDO_DINAMICO_JS] = {
        .uri_name = "/contenido_dinamico.js",
        .uri_start = contenido_dinamico_js_start,
        .uri_end = contenido_dinamico_js_end
  },
};


typedef enum {
  DYNAMIC_DUMMY_ONE,
  DYNAMIC_DUMMY_TWO,
  DYNAMIC_DUMMY_THREE,
  //Always last
  DYNAMIC_TABLE_SIZE
} dynamic_content;



char* dummy_function_one(webhandler_action_enum action, char* buffer) 
{
  switch (action) {
    case WEBHANDLER_ACTION_GET:
      snprintf (dynamic_buffer, DYNAMIC_BUFFER_SIZE, "pepino");
      return dynamic_buffer;
    case WEBHANDLER_ACTION_SET:
  }
  return NULL;
}

char* dummy_function_two(webhandler_action_enum action, char* buffer) 
{
  switch (action) {
    case WEBHANDLER_ACTION_GET:
      snprintf (dynamic_buffer, DYNAMIC_BUFFER_SIZE, "tomate");
      return dynamic_buffer;
    case WEBHANDLER_ACTION_SET:
  }
  return NULL;
}

char* dummy_function_three(webhandler_action_enum action, char* buffer) 
{
  switch (action) {
    case WEBHANDLER_ACTION_GET:
      snprintf (dynamic_buffer, DYNAMIC_BUFFER_SIZE, "lechuga");
      return dynamic_buffer;
    case WEBHANDLER_ACTION_SET:
  }
  return NULL;
}

const webhandler_dynamic_type dynamic_table[DYNAMIC_TABLE_SIZE] = {
  
  [DYNAMIC_DUMMY_ONE] = {
        .id = 1,
        .content_type = text_plain,
        .callback = dummy_function_one
  },
  [DYNAMIC_DUMMY_TWO] = {
        .id = 2,
        .content_type = text_plain,
        .callback = dummy_function_two
  },  
  [DYNAMIC_DUMMY_THREE] = {
        .id = 3,
        .content_type = text_plain,
        .callback = dummy_function_three
  },
  
};


static const char *TAG = "web_handler_example";

void app_main(void)
{
    ESP_LOGI(TAG, "Starting example");
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    /* Initialize file storage */
    const char* base_path = "/data";
    ESP_ERROR_CHECK(example_mount_storage(base_path));

    /* This helper function configures Wi-Fi or Ethernet, as selected in menuconfig.
     * Read "Establishing Wi-Fi or Ethernet Connection" section in
     * examples/protocols/README.md for more information about this function.
     */
    ESP_ERROR_CHECK(example_connect());

    /* Start the file server */
    ESP_ERROR_CHECK(example_start_file_server(base_path, 
                    uri_table, 
                    URI_COUNT,
                    dynamic_table,
                    DYNAMIC_TABLE_SIZE                                       
                    ));
    ESP_LOGI(TAG, "started");
}
