#ifndef CRYSTAL_INTERNAL_HPP
#define CRYSTAL_INTERNAL_HPP

#include "modules/crystal/CRYSTAL.h"

struct CRYSTALwindow {
    CRYSTALnative native;
    void* udata;
    CRYSTALwindowClosingCallback closingCallback;
};

#endif /* CRYSTAL_INTERNAL_HPP */
