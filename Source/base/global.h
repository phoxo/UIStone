#pragma once

// ImageStone library
#include "image_stone/ImageStone.h"

extern "C" IMAGE_DOS_HEADER __ImageBase;

#define _UISTONE_BEGIN  namespace uistone {
#define _UISTONE_END        }

//-------------------------------------------------------------------------------------
/// Auto init COM.
class FCAutoInitializeCOM
{
private:
    HRESULT   m_hr = ::CoInitialize(0);
public:
    ~FCAutoInitializeCOM() { if (SUCCEEDED(m_hr)) { ::CoUninitialize(); } }
};
//-------------------------------------------------------------------------------------
// headers
#include "dpi.h"
#include "file.h"
#include "performance_test.h"
#include "string_helper.h"
#include "message_window.h"
#include "timer.h"
#include "track_mouse_hover.h"
#include "language_text.h"
#include "language_section_loader.h"
#include "font.h"
#include "ui_event_observer.h"
#include "sequence_image.h"
#include "gdi_bitmap.h"
#include "window.h"
