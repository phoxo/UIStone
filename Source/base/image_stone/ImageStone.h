/*
    Copyright (C) =PhoXo= Fu Li

    Author   :  Fu Li
    Create   :
    Home     :  http://www.phoxo.com
    Mail     :  atphoxo@gmail.com

    This file is part of PhoXo

    This code is licensed under the terms of the MIT license.
*/
#pragma once

// core
#include "include/image.h"
#include "include/image_effect_parallel.h"
#include "include/image_handle.h"
#include "include/post_implement.h"

// codec
#include "codec_gdiplus/codec_gdiplus.h"
#include "codec_wic/codec_wic.h"

// process
#include "effect/basic.h"
#include "effect/premultiply.h"
#include "effect/rotate_90_270.h"
#include "effect/mirror_flip.h"
#include "effect/composite.h"

#include "effect/gaussian_blur.h"
#include "effect/unsharp_mask.h"
#include "effect/shadow.h"

#include "effect/clipboard.h"
#include "effect/resize_gdiplus.h"
#include "effect/resize_wic.h"

#include "effect/distort_base.h"

#include "d2d/svg_drawer.h"

class CImageStoneInitializer
{
private:
    static inline ULONG_PTR   m_token = 0;
    static inline HRESULT   m_COM_result = E_INVALIDARG;

public:
    static void Init()
    {
        m_COM_result = ::CoInitialize(0); // init COM

        Gdiplus::GdiplusStartupInput   si;
        Gdiplus::GdiplusStartup(&m_token, &si, NULL); // init GDIPLUS

        CWICFunc::CreateWICFactory(); // init WIC
        CWICInstalledCodec::Init();
    }

    static void Uninit()
    {
        CWICFunc::g_factory = NULL;
        Gdiplus::GdiplusShutdown(m_token);
        if (SUCCEEDED(m_COM_result))
        {
            ::CoUninitialize();
        }
    }
};
