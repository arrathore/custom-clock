idf_component_register(SRCS "clock.c"
                     "display.c"
                     "face.c"
                     "main.c"
                     "page_server.c"
                     "server.c"
                     "storage.c"
                     "theme.c"
                     "wifi.c"
                    INCLUDE_DIRS "../include")

littlefs_create_partition_image(
    web
    "${CMAKE_SOURCE_DIR}/web/www"
    FLASH_IN_PROJECT
)

