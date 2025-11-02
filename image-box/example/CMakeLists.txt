# example/
add_executable(image_box_example                            # create an executable from file
    main.cpp
)

target_link_libraries(image_box_example PRIVATE             # resolve linkage by linking the used lib in main.cpp
    image_box
)

# Copy entire images directory to build location
add_custom_command(
    TARGET image_box_example POST_BUILD                     # associate custom command to target and make it to be run post build (after the successfull build)
    # specifies the command to run
    COMMAND ${CMAKE_COMMAND} -E copy_directory              # path to the cmake command executable and it will run it's build in command (script) for directory copy
        ${CMAKE_CURRENT_SOURCE_DIR}/images                  # source dir to be copied
        $<TARGET_FILE_DIR:image_box_example>/images         # it copies the files into a folder named images inside the build output directory of the image_box_example executable
        # is uses a CMake generator expression that expands to the directory containing the built executable.
    COMMENT "Copying images directory to build directory"
)
