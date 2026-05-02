#ifndef CRYSTAL_LINUX_H
#define CRYSTAL_LINUX_H
#include "CRYSTAL.h"
#ifdef __cplusplus
extern "C" {
#endif

typedef CATALYST_BYTE CRYSTAL_DISPLAY_SERVER;
enum {
    CRYSTAL_DISPLAY_SERVER_NONE     = 0x00,
    CRYSTAL_DISPLAY_SERVER_X11      = 0x01,
    CRYSTAL_DISPLAY_SERVER_WAYLAND  = 0x02
};

CRYSTAL_API CRYSTAL_DISPLAY_SERVER crystalGetDisplayServer(CATALYST_RESULT* result);

#ifdef __cplusplus
} /* extern C */
#endif
#endif /* CRYSTAL_LINUX_H */
