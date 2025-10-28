# src/
add_library(image_box STATIC
    image.cpp
)

target_include_directories(image_box PUBLIC
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/external
)