#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_err.h"
#include "nvs_flash.h"
#include "protocol_examples_common.h"
#include "web_handler.h"

static const char *TAG = "web_handler_example";


typedef enum {
  INDEX_HTML,
  CONTENIDO_HTML,
  FAVICON_ICO,
  WEB_TOOL_JS,
	LOG_CSV,
  //Always last
  URI_COUNT
} uris_enum;



extern const unsigned char index_html_start[] asm("_binary_index_html_start");
extern const unsigned char index_html_end[] asm("_binary_index_html_end");
extern const unsigned char contenido_html_start[] asm("_binary_contenido_html_start");
extern const unsigned char contenido_html_end[] asm("_binary_contenido_html_end");
extern const unsigned char favicon_ico_start[] asm("_binary_favicon_ico_start");
extern const unsigned char favicon_ico_end[]   asm("_binary_favicon_ico_end");
extern const unsigned char web_tool_js_start[] asm("_binary_web_tool_js_start");
extern const unsigned char web_tool_js_end[]   asm("_binary_web_tool_js_end");

char *log_csv_generator();


const wh_uri_type uri_table[URI_COUNT] = {

  [INDEX_HTML] = {
        .uri_name = "/index.html",
        .uri_start = index_html_start,
        .uri_end = index_html_end,
				.uri_generator_callback = NULL,
  },
  [CONTENIDO_HTML] = {
        .uri_name = "/contenido.html",
        .uri_start = contenido_html_start,
        .uri_end = contenido_html_end,
				.uri_generator_callback = NULL,
  },
 [FAVICON_ICO] = {
        .uri_name = "/favicon.ico",
        .uri_start = favicon_ico_start,
        .uri_end = favicon_ico_end,
				.uri_generator_callback = NULL,
  },
  [WEB_TOOL_JS] = {
        .uri_name = "/web_tool.js",
        .uri_start = web_tool_js_start,
        .uri_end = web_tool_js_end,
				.uri_generator_callback = NULL,
  },
	[LOG_CSV] = {
        .uri_name = "/log.csv",
        .uri_start = NULL,
        .uri_end = NULL,
				.uri_generator_callback = log_csv_generator,
  },
};


typedef enum {
  DYNAMIC_DUMMY_ONE,
  DYNAMIC_DUMMY_TWO,
  DYNAMIC_DUMMY_THREE,
	DYNAMIC_DUMMY_FOUR,
	DYNAMIC_DUMMY_FIVE,
	DYNAMIC_DUMMY_SIX,
  //Always last
  DYNAMIC_TABLE_SIZE
} dynamic_content;



char* label_one_generator(webhandler_action_enum action, char* buffer) 
{
  switch (action) {
    case WEBHANDLER_ACTION_GET:
      snprintf (buffer, WEB_HANDLER_BUFFER_SIZE, "pepino");
      return buffer;
    case WEBHANDLER_ACTION_SET:
			ESP_LOGI(__FUNCTION__, "%s", buffer);
  }
  return NULL;
}

char* label_two_generator(webhandler_action_enum action, char* buffer) 
{
  switch (action) {
    case WEBHANDLER_ACTION_GET:
      snprintf (buffer, WEB_HANDLER_BUFFER_SIZE, "tomate");
      return buffer;
    case WEBHANDLER_ACTION_SET:
			ESP_LOGI(__FUNCTION__, "%s", buffer);
  }
  return NULL;
}

char* select_one_generator(webhandler_action_enum action, char* buffer) 
{
	char options[]= "Oranges\nApples\nLemmons\nStrawberries\n";
	static int selected = 4;
	
  switch (action) {
    case WEBHANDLER_ACTION_GET:
      //snprintf (buffer, WEB_HANDLER_BUFFER_SIZE, "{\"opciones\": [{ \"value\": \"1\", \"text\": \"Peras\" },{ \"value\": \"2\", \"text\": \"Bananas\" },{ \"value\": \"3\", \"text\": \"Naranjas\" }],\"sel\": \"3\"}");
      //return buffer;
	
			return webhandler_send_options_select(buffer, options, selected);

    case WEBHANDLER_ACTION_SET:
			ESP_LOGI(__FUNCTION__, "%s", buffer);
			sscanf(buffer,"%d",&selected);	
  }
  return NULL;
}

char* select_two_generator(webhandler_action_enum action, char* buffer) 
{
	char options[]= "Garlic\nPepper\nBasil\n";
	static int selected = 2;

  switch (action) {
    case WEBHANDLER_ACTION_GET:
			//snprintf (buffer, WEB_HANDLER_BUFFER_SIZE, "{\"opciones\": [{ \"value\": \"1\", \"text\": \"Peras\" },{ \"value\": \"2\", \"text\": \"Bananas\" },{ \"value\": \"3\", \"text\": \"Naranjas\" }],\"sel\": \"3\"}");
			//return buffer;
			return webhandler_send_options_select(buffer, options, selected);
    case WEBHANDLER_ACTION_SET:
			ESP_LOGI(__FUNCTION__, "%s", buffer);
			sscanf(buffer,"%d",&selected);	
  }
  return NULL;
}
	
char* checkbox_one_generator(webhandler_action_enum action, char* buffer) 
{
  switch (action) {
    case WEBHANDLER_ACTION_GET:
      snprintf (buffer, WEB_HANDLER_BUFFER_SIZE, "{\"checked\": false}");
			return buffer;
    case WEBHANDLER_ACTION_SET:
			ESP_LOGI(__FUNCTION__, "%s", buffer);
  }
  return NULL;
}


char* checkbox_two_generator(webhandler_action_enum action, char* buffer) 
{
  switch (action) {
    case WEBHANDLER_ACTION_GET:
      snprintf (buffer, WEB_HANDLER_BUFFER_SIZE, "{\"checked\": true}");
			return buffer;
    case WEBHANDLER_ACTION_SET:
			ESP_LOGI(__FUNCTION__, "%s", buffer);
  }
  return NULL;
}
	
const webhandler_dynamic_type dynamic_table[DYNAMIC_TABLE_SIZE] = {
  
  [DYNAMIC_DUMMY_ONE] = {
        .id = 1,
				.html_type = WEBHANDLER_HTML_TYPE_PLAIN,
        .content_type = text_plain,
        .callback = label_one_generator
  },
  [DYNAMIC_DUMMY_TWO] = {
        .id = 2,
				.html_type = WEBHANDLER_HTML_TYPE_PLAIN,
        .content_type = text_plain,
        .callback = label_two_generator
  },  
  [DYNAMIC_DUMMY_THREE] = {
        .id = 3,
				.html_type = WEBHANDLER_HTML_TYPE_SELECT,
        .content_type = text_plain,
        .callback = select_one_generator
  },
  [DYNAMIC_DUMMY_FOUR] = {
        .id = 4,
				.html_type = WEBHANDLER_HTML_TYPE_SELECT,
        .content_type = text_plain,
        .callback = select_two_generator
  },
  [DYNAMIC_DUMMY_FIVE] = {
        .id = 5,
				.html_type = WEBHANDLER_HTML_TYPE_CHECKBOX,
        .content_type = text_plain,
        .callback = checkbox_one_generator
  },
  [DYNAMIC_DUMMY_SIX] = {
        .id = 6,
				.html_type = WEBHANDLER_HTML_TYPE_CHECKBOX,
        .content_type = text_plain,
        .callback = checkbox_two_generator
  },	
	
};


char *log_csv_generator()
{
	static int i=0;
	static char buffer[64];
	
	if (i==10)
		return NULL;
	sprintf(buffer,"line %d\n",i);
	ESP_LOGI(TAG, "%s", buffer);
 //while(1) {;}
	i++;
	return buffer;
}



void app_main(void)
{
    ESP_LOGI(TAG, "Starting example");
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    /* Initialize file storage */
    const char* base_path = "/data";
    //ESP_ERROR_CHECK(example_mount_storage(base_path));

    /* This helper function configures Wi-Fi or Ethernet, as selected in menuconfig.
     * Read "Establishing Wi-Fi or Ethernet Connection" section in
     * examples/protocols/README.md for more information about this function.
     */
    ESP_ERROR_CHECK(example_connect());

    /* Start the file server */
    web_handler_init(base_path, 
                    uri_table, 
                    URI_COUNT,
                    dynamic_table,
                    DYNAMIC_TABLE_SIZE                                       
                    );

}
