/* DO NOT EDIT THIS FILE - it is machine generated */
#include <native.h>

#ifndef _Included_java_net_SocketInputStream
#define _Included_java_net_SocketInputStream

#ifdef __cplusplus
extern "C" {
#endif

/* Header for class java_net_SocketInputStream */

typedef struct Classjava_net_SocketInputStream {
  struct Hjava_io_FileDescriptor* fd;
  jbool eof;
  struct Hjava_net_SocketImpl* impl;
  HArrayOfByte* temp;
} Classjava_net_SocketInputStream;
HandleTo(java_net_SocketInputStream);

extern jint java_net_SocketInputStream_socketRead(struct Hjava_net_SocketInputStream*, HArrayOfByte*, jint, jint);

#ifdef __cplusplus
}
#endif

#endif
