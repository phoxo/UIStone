#pragma once
#include "define.h"
#include "image_effect.h"

/**
    @brief Image object.

    (0,0) located at <span style='color:#FF0000'>upper-left</span>, x increase from left to right, y increase from top to down.
*/
class FCImage
{
public:
    enum
    {
        PremultipliedAlpha = 0x01,
    };

private:
    int   m_attribute = 0;
    int   m_width = 0;
    int   m_height = 0;
    int   m_bpp = 0;
    BYTE   * m_pixel = nullptr;
    BYTE   ** m_line_ptr = nullptr; // line-pointer, array from top to bottom
    HBITMAP   m_DIB_Handle = nullptr;

public:
    /// @name Constructor.
    ///@{
    /***/
    /// default constructor.
    FCImage() {}
    /// copy constructor.
    FCImage(const FCImage& img) { *this = img; }
    /// move constructor.
    FCImage(FCImage&& img)
    {
        SwapImage(img);
    }
    /// destructor.
    virtual ~FCImage() { Destroy(); }
    /// copy operator.
    FCImage& operator= (const FCImage& img)
    {
        if (&img == this) { assert(false); return *this; }
        if (!img) { Destroy(); return *this; }
        if (Create(img.Width(), img.Height(), img.ColorBits(), img.m_attribute))
        {
            memcpy(GetMemStart(), img.GetMemStart(), img.GetPixelBufferSize());
        }
        return *this;
    }
    /// move assignment operator.
    FCImage& operator=(FCImage&& other)
    {
        if (&other == this) { assert(false); return *this; }
        Destroy();
        SwapImage(other);
        return *this;
    }
    ///@}

    /// @name Create / Destroy.
    ///@{
    /***/
    /// create image, bpp can be <span style='color:#FF0000'>8 , 24 , 32</span>.
    bool Create(int width, int height, int bpp, int attribute = 0)
    {
        Destroy();
        if (!width || !height || !bpp)
        {
            assert(false); return false;
        }

        m_attribute = attribute;
        m_width = width;
        m_height = abs(height);
        m_bpp = bpp;
        AllocPixelBuffer();
        if (!m_pixel || (bpp == 1) || (bpp == 4) || (bpp == 16)) // unsupported format
        {
            Destroy();
            assert(false); return false;
        }
        AllocLinePtr();
        return true;
    }

    /// destroy image.
    void Destroy()
    {
        if (m_line_ptr) { delete[] m_line_ptr; }
        if (m_DIB_Handle) { DeleteObject(m_DIB_Handle); }
        InitMember();
    }

    /// swap current image with img.
    void SwapImage(FCImage& img)
    {
        std::swap(img.m_attribute, m_attribute);
        std::swap(img.m_width, m_width);
        std::swap(img.m_height, m_height);
        std::swap(img.m_bpp, m_bpp);
        std::swap(img.m_pixel, m_pixel);
        std::swap(img.m_line_ptr, m_line_ptr);
        std::swap(img.m_DIB_Handle, m_DIB_Handle);
    }

    /// releases ownership of its DIB handle.
    HBITMAP Detach()
    {
        auto   bmp = m_DIB_Handle;
        if (m_line_ptr) { delete[] m_line_ptr; }
        InitMember();
        return bmp;
    }
    ///@}

    /// @name Attributes.
    ///@{
    /***/
    bool IsValid() const { return (m_pixel != 0); }
    bool IsInside(int x, int y) const { return (x >= 0) && (x < m_width) && (y >= 0) && (y < m_height); }
    bool IsInside(POINT pt) const { return IsInside(pt.x, pt.y); }

    /// this function doesn't do boundary check, so <span style='color:#FF0000'>Crash</span> if y exceed.
    BYTE* GetBits(int y) const
    {
        assert(IsInside(0, y));
        return m_line_ptr[y];
    }
    BYTE* GetBits(int x, int y) const
    {
        assert(IsInside(x, y));
        if (m_bpp == 32)
            return (m_line_ptr[y] + x * 4);
        if (m_bpp == 8)
            return (m_line_ptr[y] + x);
        return (m_line_ptr[y] + x * 3); // 24bpp
    }
    BYTE* GetBits(POINT pt) const { return GetBits(pt.x, pt.y); }

    SIZE GetSize() const { return CSize(m_width, m_height); }
    int Width() const { return m_width; }
    int Height() const { return m_height; }
    int ColorBits() const { return m_bpp; }
    /// bytes every scan line (the value is upper 4-bytes rounded).
    int GetStride() const { return 4 * ((m_width * m_bpp + 31) / 32); }
    /// equal stride * height
    int GetPixelBufferSize() const { return (GetStride() * Height()); }
    /// get start address of pixel.
    void* GetMemStart() const { return m_pixel; }
    int GetAttribute() const { return m_attribute; }
    operator HBITMAP() const { return m_DIB_Handle; }
    operator bool() const { return (m_pixel != 0); }
    bool IsPremultiplied() const { return (m_attribute & PremultipliedAlpha); }
    void SetPremultiplied(bool v) { v ? ModifyAttribute(0, PremultipliedAlpha) : ModifyAttribute(PremultipliedAlpha, 0); }
    ///@}

    /// @name Temporary object.
    ///@{
    /***/
    /// you can apply some effect (e.g. FCEffectBrightnessContrast) on buffer
    void Attach32bppBuffer(int width, int height, BYTE* pixel)
    {
        if ((width > 0) && (height != 0) && pixel)
        {
            Destroy();
            m_width = width;
            m_height = abs(height);
            m_bpp = 32;
            m_pixel = pixel;
            AllocLinePtr();
        }
        else { assert(false); }
    }

    void Release32bppBuffer()
    {
        if (m_line_ptr) { delete[] m_line_ptr; }
        InitMember();
    }
    ///@}

    /// @name Process.
    ///@{
    /***/
    /// apply an effect, more detail refer to FCImageEffect.
    void ApplyEffect(FCImageEffect& effect, FCProgressObserver* progress = nullptr)
    {
        if (!effect.IsSupport(*this)) { assert(false); return; }

        // before
        effect.OnBeforeProcess(*this);
        if (progress)
            progress->UpdateProgress(0);

        if (effect.QueryProcessMode() == FCImageEffect::ProcessMode::EntireMyself)
        {
            effect.ProcessEntire(*this, progress);
        }
        else
        {
            if (effect.IsParallelAccelerateEnable())
            {
                ApplyEffectParallel(effect, progress);
            }
            else
            {
                ApplyEffectSingleThread(effect, progress);
            }
        }

        effect.OnAfterProcess(*this);
        if (progress && !progress->IsUserCanceled())
            progress->UpdateProgress(100);
    }
    ///@}

private:
    void ModifyAttribute(int remove, int add) { m_attribute = ((m_attribute & ~remove) | add); }

    void InitMember()
    {
        m_attribute = 0; m_width = 0; m_height = 0; m_bpp = 0;
        m_pixel = nullptr; m_line_ptr = nullptr;
        m_DIB_Handle = nullptr;
    }

    void AllocPixelBuffer()
    {
        size_t   info_byte = sizeof(BITMAPINFOHEADER) + 16;
        if (ColorBits() <= 8)
        {
            info_byte += (sizeof(RGBQUAD) * 256);
        }

        std::vector<BYTE>   buf(info_byte, (BYTE)0);
        auto   info = (BITMAPINFO*)buf.data();

        FCBaseHelper::InitBitmapInfoHeader(info->bmiHeader, m_width, m_height, m_bpp, TRUE);
        m_DIB_Handle = CreateDIBSection(NULL, info, DIB_RGB_COLORS, (VOID**)&m_pixel, NULL, 0);
    }

    void AllocLinePtr()
    {
        m_line_ptr = (BYTE**) new BYTE[sizeof(BYTE*) * Height()];

        int   stride = GetStride();
        BYTE   * curr = m_pixel;
        for (int y = 0; y < Height(); y++, curr += stride)
        {
            m_line_ptr[y] = curr;
        }
    }

    void ApplyEffectParallel(FCImageEffect& effect, FCProgressObserver* progress);

    void ApplyEffectSingleThread(FCImageEffect& effect, FCProgressObserver* progress)
    {
        auto   type = effect.QueryProcessMode();
        if (type == FCImageEffect::ProcessMode::PixelByPixel)
        {
            for (int y = 0; y < Height(); y++)
            {
                BYTE   * curr = GetBits(y);
                for (int x = 0; x < Width(); x++, curr += (ColorBits() / 8))
                {
                    effect.ProcessPixel(*this, x, y, (RGBA32bit*)curr);
                }

                // update progress
                if ((y % 50 == 0) && progress && !progress->UpdateProgress(y * 100 / Height()))
                    break;
            }
        }
        else if (type == FCImageEffect::ProcessMode::Region)
        {
            effect.ProcessRegion(*this, CRect(0, 0, Width(), Height()), progress);
        }
    }
};
