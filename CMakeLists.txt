cmake_minimum_required(VERSION 3.10.2)
project(bookstore)

include_directories(./third_party)
set(CMAKE_CXX_STANDARD 17)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}   -Ofast")

add_executable(code ${src_dir} src/main.cpp)


set(test_prefix "${PROJECT_SOURCE_DIR}/data")
file(GLOB tests RELATIVE "${test_prefix}" data/*)

foreach (test-name IN ITEMS ${tests})
    if (NOT IS_DIRECTORY "${test_prefix}/${test-name}")
        continue()
    endif ()
    add_test(NAME "${test-name}"
            COMMAND bash ../run_test.sh "${test_prefix}" "${test-name}" "$<TARGET_FILE:code>"
            WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/run
            )
endforeach ()

enable_testing()
