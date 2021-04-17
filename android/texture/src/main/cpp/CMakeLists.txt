cmake_minimum_required(VERSION 3.10.2)

project("tiny_engine")

add_definitions(-std=c++14)

add_library(VkLayer_khronos_validation-lib
        SHARED
        IMPORTED)

set_target_properties(VkLayer_khronos_validation-lib
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/../jniLibs/${ANDROID_ABI}/libVkLayer_khronos_validation.so)

include_directories(../../../../../third_party/glm
        ../../../../../library)

add_library(native-lib
        SHARED
        native-lib.cpp
        triangle_application.cpp
        ../../../../../library/vulkan_application.cpp
        ../../../../../library/filesystem.cpp)

target_link_libraries(native-lib
        log
        vulkan
        android
        ${VkLayer_khronos_validation-lib})