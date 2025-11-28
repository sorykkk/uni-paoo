# src/

add_library(doc_analytics STATIC
    tf-idf.cpp
)

target_include_directories(doc_analytics PUBLIC
    ${PROJECT_SOURCE_DIR}/include
)
