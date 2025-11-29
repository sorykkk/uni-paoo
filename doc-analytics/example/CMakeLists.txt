# example/

add_executable(doc_analytics_demo
    doc_analytics_demo.cpp
)

target_link_libraries(doc_analytics_demo PRIVATE
    doc_analytics
)

# Smart pointer and thread safety demonstration
add_executable(unsafe_doc_analytics_demo
    unsafe_doc_analytics_demo.cpp
)

target_link_libraries(unsafe_doc_analytics_demo PRIVATE
    doc_analytics
)


