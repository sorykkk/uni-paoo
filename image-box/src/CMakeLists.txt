# src/
add_library(image_box STATIC                # add static library
    image.cpp
    image_base.cpp
)

target_include_directories(image_box PUBLIC
    ${PROJECT_SOURCE_DIR}/include           # expose include directory to be looked into if it cannot resolve an included file
    ${PROJECT_SOURCE_DIR}/external          # expose external dir (where third party scripts are hosted)
)