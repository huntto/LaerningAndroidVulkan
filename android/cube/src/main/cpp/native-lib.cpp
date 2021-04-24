#include <jni.h>
#include <string>
#include <memory>
#include <android/native_window_jni.h>
#include <android/asset_manager_jni.h>

#include <filesystem.h>
#include "cube_application.h"

std::shared_ptr<CubeApplication> application;

extern "C"
JNIEXPORT void JNICALL
Java_com_ihuntto_android_1vulkan_cube_MainActivity_init(JNIEnv *env, jobject thiz, jobject surface) {
    if (application == nullptr) {
        auto vert_shader_code = tiny_engine::Filesystem::GetInstance().Read<char>(
                "shaders/base.vert.spv");
        auto frag_shader_code = tiny_engine::Filesystem::GetInstance().Read<char>(
                "shaders/base.frag.spv");
        ANativeWindow *native_window = ANativeWindow_fromSurface(env, surface);
        application = std::make_shared<CubeApplication>(native_window,
                                                        vert_shader_code,
                                                        frag_shader_code);
        application->Init();
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_ihuntto_android_1vulkan_cube_MainActivity_cleanup(JNIEnv *env, jobject thiz) {
    if (application != nullptr) {
        application->Cleanup();
        application = nullptr;
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_ihuntto_android_1vulkan_cube_MainActivity_setAssetManager(JNIEnv *env, jobject thiz,
                                                                jobject asset_manager_obj,
                                                                jstring data_path) {
    AAssetManager *asset_manager = AAssetManager_fromJava(env, asset_manager_obj);
    tiny_engine::Filesystem::GetInstance().Init(asset_manager);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_ihuntto_android_1vulkan_cube_MainActivity_draw(JNIEnv *env, jobject thiz) {
    if (application != nullptr) {
        application->Draw();
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_ihuntto_android_1vulkan_cube_MainActivity_rotate(JNIEnv *env,
                                                       jobject thiz,
                                                       jfloat radius,
                                                       jfloat x,
                                                       jfloat y,
                                                       jfloat z) {
    if (application != nullptr) {
        application->Rotate(radius, x, y, z);
    }
}