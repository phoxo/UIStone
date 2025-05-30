#pragma once
#include <stdexcept>
#include <ranges>

using phoxo::ImageHandler;
using phoxo::BitmapHDC;

extern "C" IMAGE_DOS_HEADER __ImageBase;

//-------------------------------------------------------------------------------------
/// Auto init COM.
class AutoComInitializer
{
private:
    HRESULT   m_hr = ::CoInitialize(0);
public:
    AutoComInitializer() { assert(SUCCEEDED(m_hr)); }
    ~AutoComInitializer() { if (SUCCEEDED(m_hr)) { ::CoUninitialize(); } }
};
//-------------------------------------------------------------------------------------
// headers
#include "helper.h"
#include "dc.h"
#include "gdiplus.h"
#include "dpi.h"
#include "file.h"
#include "regkey.h"
#include "file_save_agent.h"
#include "performance_test.h"
#include "string.h"
#include "message_window.h"
#include "timer.h"
#include "track_mouse_hover.h"
#include "language_text.h"
#include "language_text_group.h"
#include "font.h"
#include "event_observer.h"
#include "sequence_3state_builder.h"
#include "gdi_bitmap.h"
#include "window.h"
