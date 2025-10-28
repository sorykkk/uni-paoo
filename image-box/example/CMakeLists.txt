# example/
add_executable(image_box_example
    main.cpp
)

target_link_libraries(image_box_example PRIVATE
    image_box
)

# Copy entire images directory to build location
add_custom_command(
    TARGET image_box_example POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/images
        $<TARGET_FILE_DIR:image_box_example>/images
    COMMENT "Copying images directory to build directory"
)
