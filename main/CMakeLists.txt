idf_component_register( SRCS "main.c" 
                             "file_server.c"
                             "mount.c"
                        INCLUDE_DIRS "."
                        EMBED_FILES 
								"../www-root/favicon.ico" 
                                "../www-root/index.html" 
                                "../www-root/contenido.html"
                                "../www-root/contenido_dinamico.js"
                       )
