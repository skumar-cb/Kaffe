/* DO NOT EDIT THIS FILE - it is machine generated */
#include <native.h>

#ifndef _Included_java_lang_reflect_Constructor
#define _Included_java_lang_reflect_Constructor

#ifdef __cplusplus
extern "C" {
#endif

/* Header for class java_lang_reflect_Constructor */

typedef struct Classjava_lang_reflect_Constructor {
  struct Hjava_lang_Class* clazz;
  jint slot;
  HArrayOfObject* parameterTypes;
  HArrayOfObject* exceptionTypes;
} Classjava_lang_reflect_Constructor;
HandleTo(java_lang_reflect_Constructor);

extern jint java_lang_reflect_Constructor_getModifiers(struct Hjava_lang_reflect_Constructor*);
extern struct Hjava_lang_Object* java_lang_reflect_Constructor_newInstance(struct Hjava_lang_reflect_Constructor*, HArrayOfObject*);

#ifdef __cplusplus
}
#endif

#endif
