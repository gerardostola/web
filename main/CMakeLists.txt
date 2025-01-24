idf_component_register( SRCS "main.c" 
                             "web_handler.c"                            
                        INCLUDE_DIRS "."
                        EMBED_FILES 
															"../www-root/favicon.ico" 
                              "../www-root/index.html" 
                              "../www-root/contenido.html"
                              "../www-root/web_tool.js"
                       )
