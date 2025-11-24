# src/
add_library(csv_sql STATIC
    table.cpp
)

target_include_directories(csv_sql PUBLIC
    ${PROJECT_SOURCE_DIR}/include
)