if(NOT TARGET GTest::gtest_main)
    message(FATAL_ERROR "GTest::gtest_main not found. Did you add_subdirectory(googletest) BEFORE tests?")
endif()
# Создаём исполняемый файл теста
add_executable(json_test json_test.cpp)

# target_compile_options(json_test PRIVATE
#     -fsanitize=address,undefined
#     -fno-omit-frame-pointer
#     -g
# )
# target_link_options(json_test PRIVATE
#     -fsanitize=address,undefined
# )

# Линкуем: наш код + gtest
target_link_libraries(json_test PRIVATE json GTest::gtest_main)

set(RES_SOURCE_DIR "${CMAKE_SOURCE_DIR}/res")
set(RES_DEST_DIR "${CMAKE_BINARY_DIR}/tests/res")

# Собираем список файлов для отслеживания
file(GLOB_RECURSE RES_FILES LIST_DIRECTORIES false "${RES_SOURCE_DIR}/*")

# Создаём директорию назначения (если её нет)
file(MAKE_DIRECTORY "${RES_DEST_DIR}")

# Кастомная команда: копирует res/ при изменении ЛЮБОГО файла в нём
add_custom_command(
    OUTPUT "${RES_DEST_DIR}/.copied"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${RES_SOURCE_DIR}/" "${RES_DEST_DIR}/"
    COMMAND ${CMAKE_COMMAND} -E touch "${RES_DEST_DIR}/.copied"
    DEPENDS ${RES_FILES}
    COMMENT "Copying resources from ${RES_SOURCE_DIR} to ${RES_DEST_DIR}"
    VERBATIM
)

# Кастомная цель, чтобы можно было явно запустить копирование
add_custom_target(copy_resources ALL
    DEPENDS "${RES_DEST_DIR}/.copied"
)

# Зависимость: тесты зависят от копирования ресурсов
# (если у тебя цель тестов называется, например, json_test)
add_dependencies(json_test copy_resources)

# Регистрируем для ctest
include(GoogleTest)
gtest_discover_tests(json_test
    DISCOVERY_MODE POST_BUILD
    DISCOVERY_TIMEOUT 30
)