#include <jni.h>
#include <string>
#include <memory>
#include <android/native_window_jni.h>
#include <android/asset_manager_jni.h>

#include <filesystem.h>
#include "triangle_application.h"

std::shared_ptr<TextureApplication> application;

extern "C"
JNIEXPORT void JNICALL
Java_com_ihuntto_tiny_1engine_triangle_MainActivity_init(JNIEnv *env, jobject thiz, jobject surface) {
    if (application == nullptr) {
        auto vert_shader_code = tiny_engine::Filesystem::GetInstance().Read<char>(
                "shaders/base.vert.spv");
        auto frag_shader_code = tiny_engine::Filesystem::GetInstance().Read<char>(
                "shaders/base.frag.spv");
        ANativeWindow *native_window = ANativeWindow_fromSurface(env, surface);
        application = std::make_shared<TextureApplication>(native_window,
                                                           vert_shader_code,
                                                           frag_shader_code);
        application->Init();
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_ihuntto_tiny_1engine_triangle_MainActivity_cleanup(JNIEnv *env, jobject thiz) {
    if (application != nullptr) {
        application->Cleanup();
        application = nullptr;
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_ihuntto_tiny_1engine_triangle_MainActivity_setAssetManager(JNIEnv *env, jobject thiz,
                                                           jobject asset_manager_obj,
                                                           jstring data_path) {
    AAssetManager *asset_manager = AAssetManager_fromJava(env, asset_manager_obj);
    tiny_engine::Filesystem::GetInstance().Init(asset_manager);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_ihuntto_tiny_1engine_triangle_MainActivity_draw(JNIEnv *env, jobject thiz) {
    if (application != nullptr) {
        application->Draw();
    }
}