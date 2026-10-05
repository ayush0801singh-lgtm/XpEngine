add_executable(expression_engine
    main.cpp
)

target_link_libraries(expression_engine PRIVATE expr::core)
set_target_properties(expression_engine PROPERTIES CXX_EXTENSIONS OFF)

expr_enable_warnings(expression_engine)
expr_configure_executable(expression_engine)
