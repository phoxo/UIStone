#pragma once
#include "image_property_gdiplus.h"
#include "gdiplus_encode_param.h"

/// Read / Write image using Gdi+.
class FCCodecGdiplus
{
public:
    static bool Load(Gdiplus::Bitmap& src, FCImage& img, Gdiplus::PixelFormat desired_format = PixelFormat32bppARGB)
    {
        int   attr = ((desired_format == PixelFormat32bppPARGB) ? FCImage::PremultipliedAlpha : 0);
        UINT   bpp = Gdiplus::GetPixelFormatSize(desired_format);
        if (img.Create(src.GetWidth(), src.GetHeight(), bpp, attr))
        {
            Gdiplus::BitmapData   bd = { 0 };
            bd.Width = img.Width();
            bd.Height = img.Height();
            bd.Stride = img.GetStride();
            bd.PixelFormat = desired_format;
            bd.Scan0 = img.GetBits(0);

            Gdiplus::Rect   trc(0, 0, img.Width(), img.Height());
            auto   b = src.LockBits(&trc, Gdiplus::ImageLockModeRead | Gdiplus::ImageLockModeUserInputBuf, desired_format, &bd); assert(b == Gdiplus::Ok);
            src.UnlockBits(&bd);
        }
        return img.IsValid();
    }

    static bool SaveFile(PCWSTR filepath, const FCImage& img, int jpeg_quality = 0, int dpi = 0)
    {
        CImagePropertyGdiplus   prop;
        prop.m_save_jpeg_quality = jpeg_quality;
        prop.m_dpi = dpi;
        return SaveFile(filepath, img, prop);
    }

    static bool SaveFile(PCWSTR filepath, const FCImage& img, const CImagePropertyGdiplus& prop)
    {
        auto   src = FCCodecGdiplus::CreateBitmapReference(img);
        if (!src.get())
            return false;

        prop.AddAllProperty(*src);

        imagestone_inner::CGdiplusSaveParam   param(filepath, prop.m_save_jpeg_quality);
        return (src->Save(filepath, &param.m_type_CLSID, param.m_encoder_param.get()) == Gdiplus::Ok);
    }

    static std::unique_ptr<Gdiplus::Bitmap> CreateBitmapReference(const FCImage& img)
    {
        if (!img) { assert(false); return nullptr; }
        auto   format = GetPixelFormat(img);
        return std::make_unique<Gdiplus::Bitmap>(img.Width(), img.Height(), img.GetStride(), format, img.GetBits(0));
    }

private:
    static Gdiplus::PixelFormat GetPixelFormat(const FCImage& img)
    {
        switch (img.ColorBits())
        {
        case 8: return PixelFormat8bppIndexed;
        case 24: return PixelFormat24bppRGB;
        case 32: return (img.IsPremultiplied() ? PixelFormat32bppPARGB : PixelFormat32bppARGB);
        }
        return PixelFormatUndefined;
    }
};
