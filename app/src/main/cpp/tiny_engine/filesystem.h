#ifndef TINY_ENGINE_FILESYSTEM_H
#define TINY_ENGINE_FILESYSTEM_H

#include <vector>
#include <string>
#include <mutex>
#include <stdexcept>

#ifdef ANDROID

#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>

#endif

namespace tiny_engine {

class Filesystem {
public:
    ~Filesystem() {}

    static Filesystem &GetInstance() {
        static std::once_flag s_flag;
        std::call_once(s_flag, [&]() {
            instance_.reset(new Filesystem);
        });

        return *instance_;
    }

    virtual void Init(void *context);


    void Read(const std::string &filename, std::string &content);

    template<typename T>
    auto Read(const std::string &filename) -> std::vector<T> {
        std::vector<T> file_content;
#ifdef ANDROID
        if (context_ == nullptr) {
            throw std::runtime_error("Call function Init first on Android platform!");
        }
        auto   *asset_manager = static_cast<AAssetManager *>(context_);
        AAsset *file          = AAssetManager_open(asset_manager, filename.c_str(),
                                                   AASSET_MODE_BUFFER);
        size_t file_length    = AAsset_getLength(file);
        file_content.resize(file_length / sizeof(T));

        AAsset_read(file, file_content.data(), file_length);
#endif
        return file_content;
    }


private:
    Filesystem() {}

    Filesystem(Filesystem &) = delete;

    Filesystem &operator=(const Filesystem &) = delete;

    static std::unique_ptr<Filesystem> instance_;

    void *context_;
};

} // namespace tiny_engine

#endif //TINY_ENGINE_FILESYSTEM_H
