#include <jni.h>
#include <string>
#include <memory>
#include <android/native_window_jni.h>
#include <android/asset_manager_jni.h>

#include "triangle_application.h"

std::shared_ptr<TriangleApplication> application;

extern "C"
JNIEXPORT void JNICALL
Java_com_ihuntto_tiny_1engine_MainActivity_init(JNIEnv *env, jobject thiz, jobject surface) {
    if (application == nullptr) {
        ANativeWindow *native_window = ANativeWindow_fromSurface(env, surface);
        application = std::make_shared<TriangleApplication>(native_window);
        application->Init();
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_ihuntto_tiny_1engine_MainActivity_cleanup(JNIEnv *env, jobject thiz) {
    if (application != nullptr) {
        application->Cleanup();
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_ihuntto_tiny_1engine_MainActivity_setAssetManager(JNIEnv *env, jobject thiz,
                                                           jobject asset_manager,
                                                           jstring data_path) {
    // TODO: implement setAssetManager()
}

extern "C"
JNIEXPORT void JNICALL
Java_com_ihuntto_tiny_1engine_MainActivity_draw(JNIEnv *env, jobject thiz) {
    if (application != nullptr) {
        application->Draw();
    }
}