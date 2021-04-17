#include "filesystem.h"

namespace tiny_engine {

std::unique_ptr<Filesystem> Filesystem::instance_;

void Filesystem::Init(void *context) {
    context_ = context;
}

void Filesystem::Read(const std::string &filename, std::string &content) {
#ifdef ANDROID
    if (context_ == nullptr) {
        throw std::runtime_error("Call function Init first on Android platform!");
    }
    auto   *asset_manager = static_cast<AAssetManager *>(context_);
    AAsset *file          = AAssetManager_open(asset_manager, filename.c_str(), AASSET_MODE_BUFFER);
    size_t file_length    = AAsset_getLength(file);

    content.resize(file_length);

    AAsset_read(file, &content[0], file_length);
#endif
}

}
