idf_component_register( SRCS "main.c" 
                             "file_server.c"
                             "mount.c"
                        INCLUDE_DIRS "."
                        EMBED_FILES "favicon.ico" 
                                "index.html" 
                                "contenido.html"
                                "contenido_dinamico.js"
                       )
