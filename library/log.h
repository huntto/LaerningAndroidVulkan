#ifndef __LOG_H__
#define __LOG_H__

#ifdef ANDROID
#include <android/log.h>

#ifndef LOG_TAG
#define LOG_TAG __FILE__
#endif

#define LOG_PRINT(level, fmt, ...) \
    __android_log_print(level, LOG_TAG, "(%s:%u) %s(*): " fmt, \
        __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)

#define LOGV(fmt, ...) LOG_PRINT(ANDROID_LOG_VERBOSE, fmt, ##__VA_ARGS__)
#define LOGD(fmt, ...) LOG_PRINT(ANDROID_LOG_DEBUG, fmt, ##__VA_ARGS__)
#define LOGI(fmt, ...) LOG_PRINT(ANDROID_LOG_INFO, fmt, ##__VA_ARGS__)
#define LOGW(fmt, ...) LOG_PRINT(ANDROID_LOG_WARN, fmt, ##__VA_ARGS__)
#define LOGE(fmt, ...) LOG_PRINT(ANDROID_LOG_ERROR, fmt, ##__VA_ARGS__)
#define LOGF(fmt, ...) LOG_PRINT(ANDROID_LOG_FATAL, fmt, ##__VA_ARGS__)
#endif // ANDROID

#endif //__LOG_H__
