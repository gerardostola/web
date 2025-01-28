//https://github.com/DaveGamble/cJSON/tree/acc76239bee01d8e9c858ae2cab296704e52d916?tab=readme-ov-file#parsing-json

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <sys/param.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include <dirent.h>

#include "esp_err.h"
#include "esp_log.h"
#include "cJSON.h"

#include "esp_vfs.h"
#include "esp_spiffs.h"
#include "esp_http_server.h"
#include "webtool.h"

/* Max length a file path can have on storage */
#define FILE_PATH_MAX (ESP_VFS_PATH_MAX + CONFIG_SPIFFS_OBJ_NAME_LEN)
#define WEBHANDLER_MAX_CONTENT_BUFFER 512




struct file_server_data {
    /* Base path of file storage */
    char base_path[ESP_VFS_PATH_MAX + 1];

    /* Scratch buffer for user exchange */
    char scratch[WEB_HANDLER_BUFFER_SIZE];
		size_t scratch_used_buffer;
    
    const wh_uri_type *uri_table;
    size_t uri_table_size;
    
    const webhandler_dynamic_type *dynamic_table;
    size_t dynamic_table_size;    
};

static esp_err_t dynamic_get_handler(httpd_req_t *req);

static const char *TAG = "web_handler";

#define IS_FILE_EXT(filename, ext) \
    (strcasecmp(&filename[strlen(filename) - sizeof(ext) + 1], ext) == 0)

// Set HTTP response content type according to file extension 
static esp_err_t set_content_type_from_file(httpd_req_t *req, const char *filename)
{
    if (IS_FILE_EXT(filename, ".pdf")) {
      return httpd_resp_set_type(req, "application/pdf");
    }
    if (IS_FILE_EXT(filename, ".json")) {
      return httpd_resp_set_type(req, "application/json");
    }     
    if (IS_FILE_EXT(filename, ".html")) {
      return httpd_resp_set_type(req, "text/html");
    }
    if (IS_FILE_EXT(filename, ".js")) {
      return httpd_resp_set_type(req, "text/javascript");
    }        
    if (IS_FILE_EXT(filename, ".jpeg")) {
      return httpd_resp_set_type(req, "image/jpeg");
    } 
    if (IS_FILE_EXT(filename, ".ico")) {
      return httpd_resp_set_type(req, "image/x-icon");
    }
    // This is a limited set only
    // For any other type always set as plain text
    return httpd_resp_set_type(req, "text/plain");
}

// Set HTTP response content type according to enum
static esp_err_t set_content_type_from_enum(httpd_req_t *req, content_type_enum content_type)
{
    switch (content_type) {
      case application_pdf:
        return httpd_resp_set_type(req, "application/pdf");
      case text_html:
        return httpd_resp_set_type(req, "text/html");
      case text_js:
        return httpd_resp_set_type(req, "text/javascript");
      case image_jpeg:
        return httpd_resp_set_type(req, "image/jpeg");
      case image_x_icon:
        return httpd_resp_set_type(req, "image/x-icon");
      case application_json:
        return httpd_resp_set_type(req, "application/json");
      case text_plain:
      default:
        return httpd_resp_set_type(req, "text/plain");      
    }
}



/* Copies the full path into destination buffer and returns
 * pointer to path (skipping the preceding base path) */
static const char* get_path_from_uri(char *dest, const char *base_path, const char *uri, size_t destsize)
{
    const size_t base_pathlen = strlen(base_path);
    size_t pathlen = strlen(uri);

    const char *quest = strchr(uri, '?');
    if (quest) {
        pathlen = MIN(pathlen, quest - uri);
    }
    const char *hash = strchr(uri, '#');
    if (hash) {
        pathlen = MIN(pathlen, hash - uri);
    }

    if (base_pathlen + pathlen + 1 > destsize) {
        /* Full path string won't fit into destination buffer */
        return NULL;
    }

    /* Construct full path (base + path) */
    strcpy(dest, base_path);
    strlcpy(dest + base_pathlen, uri, pathlen + 1);

    /* Return pointer to path, skipping the base */
    return dest + base_pathlen;
}

/* Handler to GET method */
static esp_err_t wh_get_handler(httpd_req_t *req)
{
    char filepath[FILE_PATH_MAX];
    struct file_server_data *server_data = (struct file_server_data *)req->user_ctx;
    
    const char *filename = get_path_from_uri(filepath, ((struct file_server_data *)req->user_ctx)->base_path,
                                             req->uri, sizeof(filepath));
    if (!filename) {
      ESP_LOGE(TAG, "Filename is too long");
      /* Respond with 500 Internal Server Error */
      httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Filename too long");
      return ESP_FAIL;
    }    
    ESP_LOGI(__FUNCTION__, "%s", filename);

    if (strcmp(filename, "/dyn") == 0) {
      return dynamic_get_handler(req);
    } 

    for (int i=0; i<server_data->uri_table_size;i++) {
      //ESP_LOGI(__FUNCTION__, "compare with  %s", server_data->uri_table[i].uri_name);
      if (strcmp(filename, server_data->uri_table[i].uri_name) == 0) {

        if (ESP_OK==set_content_type_from_file(req, filename)) {
					
					
					if ( NULL!=server_data->uri_table[i].uri_generator_callback) {
						httpd_resp_set_hdr(req, "Content-Disposition", "attachment");
					
						char *data_to_send;
						char *data_to_send_copy=NULL;
						do {
							data_to_send = server_data->uri_table[i].uri_generator_callback();
							data_to_send_copy=data_to_send;
							if (NULL!=data_to_send) {
								httpd_resp_send_chunk(req, data_to_send, strlen(data_to_send));
							}
							
						} while (data_to_send_copy!=NULL);
				
					  httpd_resp_send_chunk(req, data_to_send, 0);
			
					}
					else
					{
						size_t uri_size = server_data->uri_table[i].uri_end - server_data->uri_table[i].uri_start;
						httpd_resp_send(req, (const char*) server_data->uri_table[i].uri_start, uri_size);
          }
					return ESP_OK;
        }
        /* unknown format */
        httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "Error");
      }
    }
        
    /* Respond with 404 Not Found */
    httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "Error");
    return ESP_FAIL;
}


/* Handler for dynamic GET */
static esp_err_t dynamic_get_handler(httpd_req_t *req)
{
  int id;
  struct file_server_data *server_data = (struct file_server_data *)req->user_ctx;
  
  //size_t returned_data_size, buff_size;
  //char buff[WEBHANDLER_MAX_CONTENT_BUFFER];
  
  ESP_LOGI(__FUNCTION__, "%s",req->uri);	 
  if (1==sscanf (req->uri,"/dyn?id=%u", &id)) {  
  
	  for (int i=0; i<server_data->dynamic_table_size;i++) {
	//	  ESP_LOGI(__FUNCTION__, "i=%d", i);	
		  if (id==server_data->dynamic_table[i].id) {

				if (ESP_OK==set_content_type_from_enum(req, server_data->dynamic_table[i].content_type)) {
					if (NULL==server_data->dynamic_table[i].callback) {
						ESP_LOGE(__FUNCTION__, "callback undefined");	
						//goto dynamic_get_handler_end;
						httpd_resp_send(req, (const char*) server_data->scratch, 0);
					}
					else {
						int returned_data_size = server_data->dynamic_table[i].callback(WEBHANDLER_ACTION_GET, 
																																						server_data->scratch,
																																						WEB_HANDLER_BUFFER_SIZE);
						if (returned_data_size>0) {
							httpd_resp_send(req, (const char*) server_data->scratch, returned_data_size);
						}
						else {
							httpd_resp_send(req, (const char*) server_data->scratch, 0);
						}
						return ESP_OK;
					}
				}
				goto dynamic_get_handler_end;
		  }
	  }
  }

dynamic_get_handler_end:  
  httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Error");
  ESP_LOGE(__FUNCTION__, "ERROR");
  return ESP_FAIL;
}




void parse_object(cJSON *item)
{
	cJSON *subitem=item->child;
	while (subitem)
	{
		// handle subitem
		if (subitem->child) {
			parse_object(subitem->child);
		}

		subitem=subitem->next;
	}
}

/* Handler for POSTs */
static esp_err_t wh_post_handler(httpd_req_t *req)
{
  esp_err_t	result = ESP_FAIL;
  //char buff[WEBHANDLER_MAX_CONTENT_BUFFER];
  //size_t buff_size;
  cJSON *root = NULL;
  cJSON *set = NULL;
  cJSON *item = NULL;
	int i;
	
  struct file_server_data *server_data;


  char filepath[FILE_PATH_MAX];
  server_data = (struct file_server_data *)req->user_ctx;
  const char *filename = get_path_from_uri(filepath, ((struct file_server_data *)req->user_ctx)->base_path,
                                             req->uri, sizeof(filepath));
  if (!filename) {
      ESP_LOGE(TAG, "Filename is too long");
      /* Respond with 500 Internal Server Error */
      httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Filename too long");
      return ESP_FAIL;
    }    
    ESP_LOGI(__FUNCTION__, "%s", filename);

    
    ESP_LOGI(__FUNCTION__, "%s", req->uri);

    ESP_LOGI(__FUNCTION__, "content length %d", req->content_len);

    if (req->content_len>=WEBHANDLER_MAX_CONTENT_BUFFER) {
      httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Error");
			ESP_LOGE(__FUNCTION__, "buffer too small (%d)", __LINE__);
      return ESP_FAIL;
    }



    int received = httpd_req_recv(req, server_data->scratch, req->content_len);
    if (received <= 0) {
      httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Error");
			ESP_LOGE(__FUNCTION__, "httpd_resp_send_err %d", __LINE__);
      return ESP_FAIL;
    }
    server_data->scratch[received]=0;
    
   ESP_LOGI(__FUNCTION__, "content %s",  server_data->scratch);
		 
	 root=cJSON_Parse( server_data->scratch);
	    
  if (root == NULL) {    
    const char *error_ptr = cJSON_GetErrorPtr();

    if (error_ptr != NULL) {
      ESP_LOGE(__FUNCTION__, "cJSON_Parse %s (%d)", error_ptr, __LINE__);
    }
		else {
      ESP_LOGE(__FUNCTION__, "cJSON_Parse (%d)", __LINE__);
		}
		httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Error");
	  goto handler_end;
  }

	if (strcmp(filename, "/dyn") == 0) {
		server_data->scratch_used_buffer=0;
		cJSON *id = cJSON_GetObjectItemCaseSensitive(root, "get");
		if (NULL==id) {
			httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Error");
			ESP_LOGE(__FUNCTION__, "id NULL (%d)", __LINE__);
			goto handler_end;
		}
		server_data->scratch_used_buffer = sprintf (server_data->scratch, "[");
		cJSON_ArrayForEach(item, id) {
			if (NULL==item || !cJSON_IsNumber(item)) {
				httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Error");
				ESP_LOGE(__FUNCTION__, "item NaN %d",__LINE__);
				goto handler_end;
			}
			//ESP_LOGI(__FUNCTION__, "id %d", item->valueint);

			
			//find callback for each item
			for (i=0; i<server_data->dynamic_table_size;i++) {
				//ESP_LOGI(__FUNCTION__, "server_data->dynamic_table[i].id %d", server_data->dynamic_table[i].id);
				if (item->valueint==server_data->dynamic_table[i].id) {
					if (NULL==server_data->dynamic_table[i].callback) {
						httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Error");
						ESP_LOGE(__FUNCTION__, "callback NULL (%d)",__LINE__);
						goto handler_end;
					}
					server_data->scratch_used_buffer += snprintf (server_data->scratch+server_data->scratch_used_buffer,
												WEB_HANDLER_BUFFER_SIZE-server_data->scratch_used_buffer,
												"{\"id\":%d,",
												item->valueint);
												
					if (server_data->scratch_used_buffer>=WEB_HANDLER_BUFFER_SIZE) {
						httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Error");
						ESP_LOGE(__FUNCTION__, "buffer too small (%d)",__LINE__);
						goto handler_end;
					}
												
					size_t returned_data = server_data->dynamic_table[i].callback(WEBHANDLER_ACTION_GET,
																									server_data->scratch+server_data->scratch_used_buffer,
																									WEB_HANDLER_BUFFER_SIZE-server_data->scratch_used_buffer);
					if (0==returned_data) {
						httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Error");
						ESP_LOGE(__FUNCTION__, "callback error (%d)", __LINE__);
						goto handler_end;
					}
					server_data->scratch_used_buffer+= returned_data;
					
					server_data->scratch_used_buffer += snprintf (server_data->scratch+server_data->scratch_used_buffer,
							WEB_HANDLER_BUFFER_SIZE-server_data->scratch_used_buffer,
							"},");
												
					if (server_data->scratch_used_buffer>=WEB_HANDLER_BUFFER_SIZE) {
						httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Error");
						ESP_LOGE(__FUNCTION__, "buffer too small (%d)",__LINE__);
						goto handler_end;
					}

					break;  //for
				}
			}
			if (i==server_data->dynamic_table_size) {
				httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Error");
				ESP_LOGE(__FUNCTION__, "id not found (%d)",__LINE__);
				goto handler_end;
			}
		}
		
   	/* Redirect onto root to see the updated file list */
		httpd_resp_set_status(req, "200 OK");
		//httpd_resp_set_hdr(req, "Location", req->uri);

		set_content_type_from_enum(req, application_json);
		sprintf (server_data->scratch+server_data->scratch_used_buffer-1,"]");								
		ESP_LOGI(__FUNCTION__, "bueno: %s", server_data->scratch);

		httpd_resp_send(req, (const char*) server_data->scratch, server_data->scratch_used_buffer);
	}
	else {
		set = cJSON_GetObjectItemCaseSensitive(root, "set");
		cJSON_ArrayForEach(item, set)
		{
			cJSON *id = cJSON_GetObjectItemCaseSensitive(item, "id");
			cJSON *value = cJSON_GetObjectItemCaseSensitive(item, "value");

			if (NULL==id || !cJSON_IsNumber(id)) {
				httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Error");
				ESP_LOGE(__FUNCTION__, "id NaN (%d)",__LINE__);
				goto handler_end;
			}
			if (cJSON_IsString(value) && (value->valuestring == NULL)) {
				httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Error");
				ESP_LOGE(__FUNCTION__, "value string error (%d)", __LINE__);
				goto handler_end;				
			}
			ESP_LOGI(__FUNCTION__, "id: %d value, %s", id->valueint, value->valuestring);
			
			//find callback for this id
			for (int i=0; i<server_data->dynamic_table_size;i++) {
				if (id->valueint==server_data->dynamic_table[i].id) {
					if (NULL==server_data->dynamic_table[i].callback) {
						httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Error");
						ESP_LOGE(__FUNCTION__, "callback NULL (%d)",__LINE__);
						goto handler_end;
					}
					/*
					if (WEBHANDLER_HTML_TYPE_SELECT ==server_data->dynamic_table[i].html_type) {
						int index;
						sscanf(value->valuestring, "%d", &index);
						sprintf (value->valuestring, "%d", index);
					}
					*/
					server_data->dynamic_table[i].callback(WEBHANDLER_ACTION_SET, value->valuestring, strlen(value->valuestring));
					break; //for
				}
			}
		}
		
		/* Redirect onto root to see the updated file list */
		httpd_resp_set_status(req, "200 OK");
		//httpd_resp_set_hdr(req, "Location", req->uri);

		set_content_type_from_enum(req, application_json);
		server_data->scratch_used_buffer=sprintf(server_data->scratch, "{\"success\": true,\"mensaje\":\"Configuracion actualizada\"}");

		httpd_resp_send(req,(const char*)server_data->scratch,server_data->scratch_used_buffer);
		//httpd_resp_sendstr_chunk(req, "");
		/* Send empty chunk to signal HTTP response completion */
		//httpd_resp_send_chunk(req, NULL, 0);
		//httpd_resp_send(req, (const char*) "", 0);
	}
	result=ESP_OK;
handler_end:
  cJSON_Delete(root);
  return result;
}


int webhandler_send_select_options(char *buffer) {
    char *field;

    field = strtok(buffer, "\n");
    while (field != NULL) {
				ESP_LOGI(TAG, "field %s", field);      
        field = strtok(NULL, "\n");
    }
    return 0;
}

size_t webtool_get_select(char *buffer, size_t buffer_size, char *options, int selected)
{
	int available_buffer = buffer_size;
	size_t used;
	char *pBuffer = buffer;
	int index = 1;  
	
	used = snprintf (pBuffer, available_buffer, "\"opt\":["); 
	if (used>=available_buffer) {
		ESP_LOGE(__FUNCTION__, "buffer too small (%d)", __LINE__);
		return 0;				
	}
	
	pBuffer+=used;
	available_buffer-=used;
	char *field;
	field = strtok(options, "\n");
	while (field != NULL) {
			used = snprintf (pBuffer, available_buffer, "{\"value\":\"%d\",\"text\":\"%s\"},", index, field);
			if (used>=available_buffer) {
				ESP_LOGE(__FUNCTION__, "buffer too small (%d)", __LINE__);
				return 0;				
			}
			pBuffer+=used;
			available_buffer-=used;
			index++;
			field = strtok(NULL, "\n");
	}
	pBuffer = pBuffer - 1;
	used = snprintf (pBuffer, available_buffer+1, "],\"sel\":\"%d\"", selected);
	if (used>=available_buffer) {
		ESP_LOGE(__FUNCTION__, "buffer too small (%d)", __LINE__);
		return 0;
	}
  pBuffer+=used;
	ESP_LOGI(__FUNCTION__, "%s", buffer);
	return (size_t) (pBuffer - buffer);
}


size_t webtool_get_checkbox(char *buffer, size_t buffer_size, bool checked)
{
	size_t used;
  
	if (checked) {
		used = snprintf (buffer, buffer_size, "\"checked\":true");
  }
	else
	{
		used = snprintf (buffer, buffer_size, "\"checked\":false");
	}
	if (used>=buffer_size) {
		ESP_LOGE(__FUNCTION__, "buffer too small (%d)", __LINE__);
		return 0;				
	}
	return used;	
}

bool webtool_set_checkbox(char *buffer, size_t buffer_size, bool *checked)
{
	bool parse_ok = false;
	
	
	ESP_LOGI(__FUNCTION__, "buffer %s size %d", buffer, buffer_size);
	
	if (3==buffer_size && buffer[0]=='o' && buffer[1]=='f' && buffer[2]=='f') {
		*checked = false;
		parse_ok=true;
		goto webtool_set_checkbox_end;
	}
	if (2==buffer_size && buffer[0]=='o' && buffer[1]=='n') {
		*checked = true;
		parse_ok=true;
	}
webtool_set_checkbox_end:	
	return parse_ok;
}


size_t webtool_get_input_text(char *buffer, size_t buffer_size, char *text)
{
	size_t used;
			
	used = snprintf (buffer, buffer_size, "\"value\":\"%s\"", text);
	if (used>=buffer_size) {
		ESP_LOGE(__FUNCTION__, "buffer too small (%d)", __LINE__);
		used = 0;
	}
	return used;
}

bool webtool_set_input_text(char *input_buffer, 
														size_t input_buffer_size, 
														char *output_buffer, 
														size_t output_buffer_size)
{
	bool parse_ok = false;
	
	
	
	
	if (output_buffer_size<input_buffer_size) {
		ESP_LOGE(__FUNCTION__, "output_buffer_size<input_buffer_size");
		goto webtool_set_input_text_end;
	}
	snprintf (output_buffer, output_buffer_size, "%s", input_buffer);
	parse_ok=true;

webtool_set_input_text_end:	
	return parse_ok;


}
														
esp_err_t web_handler_init	(const char *base_path, 
                             const wh_uri_type *uri_table, 
                             const size_t uri_table_size,
                             const webhandler_dynamic_type *dynamic_table,
                             const size_t dynamic_table_size)
{
    static struct file_server_data *server_data = NULL;

    if (server_data) {
      ESP_LOGE(TAG, "File server already started");
      return ESP_ERR_INVALID_STATE;
    }

    /* Allocate memory for server data */
    server_data = calloc(1, sizeof(struct file_server_data));
    if (!server_data) {
      ESP_LOGE(TAG, "Failed to allocate memory for server data");
      return ESP_ERR_NO_MEM;
    }
    strlcpy(server_data->base_path, base_path,
            sizeof(server_data->base_path));

    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    server_data->uri_table = uri_table;
    server_data->uri_table_size = uri_table_size;

    server_data->dynamic_table = dynamic_table;
    server_data->dynamic_table_size = dynamic_table_size;


    /* Use the URI wildcard matching function in order to
     * allow the same handler to respond to multiple different
     * target URIs which match the wildcard scheme */
    config.uri_match_fn = httpd_uri_match_wildcard;

    ESP_LOGI(TAG, "Starting HTTP Server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) != ESP_OK) {
      ESP_LOGE(TAG, "Failed to start file server!");
      return ESP_FAIL;
    }

    /* URI handler for GET  */
    httpd_uri_t get = {
      .uri       = "/*",  // Match all URIs of type /path/to/file
      .method    = HTTP_GET,
      .handler   = wh_get_handler,
      .user_ctx  = server_data    // Pass server data as context
    };
    httpd_register_uri_handler(server, &get);

    /* URI handler for POST  */
    httpd_uri_t post = {
      .uri       = "/*",  // Match all URIs of type /path/to/file
      .method    = HTTP_POST,
      .handler   = wh_post_handler,
      .user_ctx  = server_data    // Pass server data as context
    };
    httpd_register_uri_handler(server, &post);
    return ESP_OK;
}
