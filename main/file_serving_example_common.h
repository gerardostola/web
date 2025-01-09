#pragma once

#include "sdkconfig.h"
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t example_mount_storage(const char *base_path);

typedef struct {
  char *uri_name;
  const unsigned char* uri_start;
  const unsigned char* uri_end;
  size_t uri_size;
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

typedef struct {
  int id;  
  content_type_enum content_type;
  webhandler_callback callback;
} webhandler_dynamic_type;

esp_err_t example_start_file_server(const char *base_path, 
                                    const wh_uri_type *uri_table, 
                                    const size_t uri_table_size,
                                    const webhandler_dynamic_type *dynamic_table,
                                    const size_t dynamic_table_size);






#ifdef __cplusplus
}
#endif
