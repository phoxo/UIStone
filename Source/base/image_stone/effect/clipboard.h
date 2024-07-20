#pragma once
/// @cond
#include <ShlObj.h>
/// @endcond

/// Copy image to clipboard (24 or 32 bit).
class FCEffectCopyToClipboard : public FCImageEffect
{
private:
    CString   m_filepath;

public:
    FCEffectCopyToClipboard(PCWSTR filepath = L"") : m_filepath(filepath) {}

private:
    virtual ProcessMode QueryProcessMode()
    {
        return ProcessMode::EntireMyself;
    }

    static HGLOBAL CreateImageData(FCImage& img)
    {
        auto   mem = GlobalAlloc(GMEM_MOVEABLE, img.GetPixelBufferSize() + sizeof(BITMAPINFOHEADER));
        auto   ptr = (BITMAPINFOHEADER*)GlobalLock(mem);
        FCBaseHelper::InitBitmapInfoHeader(*ptr, img.Width(), img.Height(), img.ColorBits(), false);

        FCEffectFlip   flip;
        flip.EnableParallelAccelerate(true);
        img.ApplyEffect(flip);
        CopyMemory(ptr + 1, img.GetMemStart(), img.GetPixelBufferSize());
        img.ApplyEffect(flip);

        GlobalUnlock(mem);
        return mem;
    }

    HGLOBAL CreateFileObject()
    {
        if (!PathFileExists(m_filepath))
            return NULL;

        DROPFILES   dp_files = { 0 };
        dp_files.pFiles = sizeof(DROPFILES);
        dp_files.fWide = TRUE;

        int   path_bytes = m_filepath.GetLength() * 2 + 2;
        auto   mem = GlobalAlloc(GMEM_ZEROINIT | GMEM_MOVEABLE, sizeof(DROPFILES) + path_bytes + 2); // double 0
        auto   ptr = (BYTE*)GlobalLock(mem);
        *(DROPFILES*)ptr = dp_files;
        ptr += sizeof(DROPFILES);
        CopyMemory(ptr, (PCWSTR)m_filepath, path_bytes);
        GlobalUnlock(mem);
        return mem;
    }

    void CopyFileObject()
    {
        auto   fobj = CreateFileObject();
        if (!fobj)
            return;

        auto   op = GlobalAlloc(GMEM_MOVEABLE, sizeof(DWORD));
        *(DWORD*)GlobalLock(op) = DROPEFFECT_COPY;
        GlobalUnlock(op);

        SetClipboardData(CF_HDROP, fobj);
        SetClipboardData(RegisterClipboardFormat(L"Preferred DropEffect"), op);
    }

    virtual void ProcessEntire(FCImage& img, FCProgressObserver* progress)
    {
        if (::OpenClipboard(NULL))
        {
            ::EmptyClipboard();
            ::SetClipboardData(CF_DIB, CreateImageData(img));
            CopyFileObject();
            ::CloseClipboard();
        }
    }
};

/// Get image from clipboard.
class FCEffectGetClipboard : public FCImageEffect
{
    virtual bool IsSupport(const FCImage& img)
    {
        return true;
    }

    virtual ProcessMode QueryProcessMode()
    {
        return ProcessMode::EntireMyself;
    }

    virtual void ProcessEntire(FCImage& img, FCProgressObserver* progress)
    {
        if (::OpenClipboard(NULL))
        {
            auto   bmp = CWICFunc::CreateBitmapFromHBITMAP((HBITMAP)::GetClipboardData(CF_BITMAP), WICBitmapUseAlpha);
            FCCodecWIC::Load(bmp, img, WICNormal32bpp);
            ::CloseClipboard();
        }
    }
};
