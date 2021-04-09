cmake_minimum_required(VERSION 3.10.2)

project("tiny_engine")

add_library(VkLayer_khronos_validation-lib
        SHARED
        IMPORTED)
set_target_properties(VkLayer_khronos_validation-lib
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/../jniLibs/${ANDROID_ABI}/libVkLayer_khronos_validation.so)

add_library(native-lib
        SHARED
        native-lib.cpp
        triangle_application.cpp
        tiny_engine/vulkan_application.cpp
        tiny_engine/filesystem.cpp)

target_link_libraries(native-lib
        log
        vulkan
        android
        ${VkLayer_khronos_validation-lib})