# example/
add_executable(csv_sql_demo
    csv_sql_demo.cpp
)
target_link_libraries(csv_sql_demo PRIVATE
    csv_sql
)