# example/
add_executable(project1_demo                            # create an executable from file
    project1_demonstration.cpp
)

target_link_libraries(project1_demo PRIVATE             # resolve linkage by linking the used lib in main.cpp
    image_box
)

# Demonstrate usage for second project
add_executable(project2_demo
    project2_demonstration.cpp
)

target_link_libraries(project2_demo PRIVATE
    image_box
)

# Copy entire images directory to build location for both executables
add_custom_command(
    TARGET project1_demo POST_BUILD             # associate custom command to target and make it to be run post build (after the successfull build)
    # specifies the command to run
    COMMAND ${CMAKE_COMMAND} -E copy_directory  # path to the cmake command executable and it will run it's build in command (script) for directory copy
        ${CMAKE_CURRENT_SOURCE_DIR}/images      # source dir to be copied
        $<TARGET_FILE_DIR:project1_demo>/images # it copies the files into a folder named images inside the build output directory of the image_box_example executable
        # is uses a CMake generator expression that expands to the directory containing the built executable.
    COMMENT "Copying images directory to build directory"
)

# add_custom_command(
#     TARGET project2_demo POST_BUILD
#     COMMAND ${CMAKE_COMMAND} -E copy_directory
#         ${CMAKE_CURRENT_SOURCE_DIR}/images
#         $<TARGET_FILE_DIR:project2_demo>/images
#     COMMENT "Copying images directory for project2_demo"
# )
