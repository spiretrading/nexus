#ifndef NEXUS_DLL_EXPORTS
#define NEXUS_DLL_EXPORTS

#ifdef _MSC_VER
  #define NEXUS_EXTERN
  #if defined(NEXUS_BUILD_DLL)
    #define NEXUS_EXPORT_DLL __declspec(dllexport)
  #elif defined(NEXUS_USE_DLL)
    #define NEXUS_EXPORT_DLL __declspec(dllimport)
  #else
    #define NEXUS_EXPORT_DLL
  #endif
#else
  #if defined(NEXUS_BUILD_DLL) || defined(NEXUS_USE_DLL)
    #define NEXUS_EXTERN extern
    #define NEXUS_EXPORT_DLL __attribute__((visibility ("default")))
  #else
    #define NEXUS_EXTERN
    #define NEXUS_EXPORT_DLL
  #endif
#endif

#endif
