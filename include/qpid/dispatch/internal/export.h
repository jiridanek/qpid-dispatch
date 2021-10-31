/// @cond INTERNAL

/// import/export macros
#if defined(_WIN32)
#  define QD_EXPORT __declspec(dllexport)
#else
#  define QD_EXPORT __attribute__((visibility("default"))) __attribute__((used))
#endif
