/* DO NOT EDIT THIS FILE - it is machine generated */
#include <native.h>

#ifndef _Included_java_lang_reflect_Method
#define _Included_java_lang_reflect_Method

#ifdef __cplusplus
extern "C" {
#endif

/* Header for class java_lang_reflect_Method */

typedef struct Classjava_lang_reflect_Method {
  struct Hjava_lang_Class* clazz;
  jint slot;
  struct Hjava_lang_String* name;
  struct Hjava_lang_Class* returnType;
  HArrayOfObject* parameterTypes;
  HArrayOfObject* exceptionTypes;
} Classjava_lang_reflect_Method;
HandleTo(java_lang_reflect_Method);

extern jint java_lang_reflect_Method_getModifiers(struct Hjava_lang_reflect_Method*);
extern struct Hjava_lang_Object* java_lang_reflect_Method_invoke(struct Hjava_lang_reflect_Method*, struct Hjava_lang_Object*, HArrayOfObject*);

#ifdef __cplusplus
}
#endif

#endif
