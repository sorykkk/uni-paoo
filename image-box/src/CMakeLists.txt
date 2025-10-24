# src/
add_library(image_box STATIC)

target_include_directories(image_box PRIVATE
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
)

target_sources(image_box PRIVATE
    image.cpp
)

target_link_libraries(image_box PRIVATE
    image_box_settings
)