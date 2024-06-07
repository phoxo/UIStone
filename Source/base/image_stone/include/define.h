#pragma once

/// @cond
#include <windows.h>
#include <comdef.h> // for SDK app, must include this before include GDI+

#include <atltypes.h>

#include <cmath>
#include <cassert>
#include <memory>
#include <string>
#include <vector>
#include <deque>
#include <algorithm>
/// @endcond

//-------------------------------------------------------------------------------------
#define _IMSTONE_PRIVATE_BEGIN  namespace imsprivate {
#define _IMSTONE_PRIVATE_END        }

/// same as RGBQUAD, just alias, rgbReserved is so ugly
union RGBA32bit
{
    RGBQUAD   quad;
    struct { BYTE b, g, r, a; };

    operator RGBQUAD() const { return quad; }
    void operator=(const RGBQUAD& c) { quad = c; }
};

static_assert(sizeof(RGBA32bit) == 4, "same as RGBQUAD");

//-------------------------------------------------------------------------------------
#include "base_helper.h"
#include "base_image_drawdc.h"
#include "base_math.h"
#include "color.h"
#include "file_ext.h"
#include "progress_observer.h"
