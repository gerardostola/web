#pragma once

#include "sdkconfig.h"
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif


#define WEB_HANDLER_BUFFER_SIZE 512
//esp_err_t example_mount_storage(const char *base_path);

typedef char* (*webhandler_uri_generator_callback) ();
/*
typedef struct {

} wh_handle_type;

*/
typedef struct {
  char *uri_name;
  const unsigned char* uri_start;
  const unsigned char* uri_end;
  size_t uri_size;
	webhandler_uri_generator_callback uri_generator_callback;
} wh_uri_type;

typedef enum {
  application_pdf,
  text_html,
  text_js,
  image_jpeg,
  image_x_icon,
  application_json,
  text_plain
} content_type_enum;

typedef enum {
  WEBHANDLER_ACTION_SET,
  WEBHANDLER_ACTION_GET
} webhandler_action_enum;


typedef char* (*webhandler_callback) (webhandler_action_enum action, char *buffer);


typedef enum {
	WEBHANDLER_HTML_TYPE_PLAIN,
	WEBHANDLER_HTML_TYPE_SELECT,
	WEBHANDLER_HTML_TYPE_CHECKBOX
} wh_html_type_enum;

typedef struct {
  int id;  
	wh_html_type_enum html_type;
  content_type_enum content_type;
  webhandler_callback callback;
} webhandler_dynamic_type;




esp_err_t web_handler_init(const char *base_path, 
                                    const wh_uri_type *uri_table, 
                                    const size_t uri_table_size,
                                    const webhandler_dynamic_type *dynamic_table,
                                    const size_t dynamic_table_size);





char *webhandler_send_options_select(char *buffer, char *options, int selected);
#ifdef __cplusplus
}
#endif
