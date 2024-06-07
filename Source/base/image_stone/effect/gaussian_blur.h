#pragma once

_IMSTONE_PRIVATE_BEGIN
class FCBoxBlurAccumulator
{
private:
    int   m_radius;
    bool   m_copy_edge_pixel;
    bool   m_only_blur_alpha;
    bool   m_all_alpha_0xFF;
    int   m_kernal_length;

public:
    FCBoxBlurAccumulator(int radius, bool copy_edge_pixel, bool only_blur_alpha, bool all_alpha_0xFF)
    {
        m_radius = radius;
        m_copy_edge_pixel = copy_edge_pixel;
        m_only_blur_alpha = only_blur_alpha;
        m_all_alpha_0xFF = all_alpha_0xFF;
        m_kernal_length = 2 * m_radius + 1;
    }

    void VerticalGetLine(const FCImage& img, int x, RGBA32bit* curr) const
    {
        FillPadding(curr, img.GetBits(x, 0));
        for (int y = 0; y < img.Height(); y++)
        {
            *curr++ = *(RGBA32bit*)img.GetBits(x, y);
        }
        FillPadding(curr, img.GetBits(x, img.Height() - 1));
    }

    void HorizontalGetLine(const FCImage& img, int y, RGBA32bit* curr) const
    {
        FillPadding(curr, img.GetBits(y));
        CopyMemory(curr, img.GetBits(y), img.GetStride()); curr += img.Width();
        FillPadding(curr, img.GetBits(img.Width() - 1, y));
    }

    auto CreateLineBuffer(int width_or_height) const
    {
        int   buffer_pixel = width_or_height + 2 * m_radius + 4; // +4 just for safety
        return std::vector<RGBA32bit>(buffer_pixel);
    }

    void BlurScanLine(RGBA32bit* dest, int pixel_span, const RGBA32bit* line, int width_or_height) const
    {
        float   sr = 0, sg = 0, sb = 0, sa = 0;
        AccumulateFirstKernal(line, sr, sg, sb, sa);

        auto   begin = line, end = line + m_kernal_length;
        if (m_only_blur_alpha)
        {
            for (int i = 0; i < width_or_height; i++, begin++, end++, dest += pixel_span)
            {
                dest->a = (BYTE)(sa / m_kernal_length + 0.5f);
                sa += end->a - begin->a;
            }
        }
        else if (m_all_alpha_0xFF && m_copy_edge_pixel)
        {
            for (int i = 0; i < width_or_height; i++, begin++, end++, dest += pixel_span)
            {
                dest->b = (BYTE)(sb / m_kernal_length + 0.5f);
                dest->g = (BYTE)(sg / m_kernal_length + 0.5f);
                dest->r = (BYTE)(sr / m_kernal_length + 0.5f);
                dest->a = 0xFF;

                sb += end->b - begin->b;
                sg += end->g - begin->g;
                sr += end->r - begin->r;
            }
        }
        else
        {
            for (int i = 0; i < width_or_height; i++, begin++, end++, dest += pixel_span)
            {
                if (sa)
                {
                    dest->b = (BYTE)(sb / sa + 0.5f);
                    dest->g = (BYTE)(sg / sa + 0.5f);
                    dest->r = (BYTE)(sr / sa + 0.5f);
                    dest->a = (BYTE)(sa / m_kernal_length + 0.5f);
                }
                else
                {
                    *(uint32_t*)dest = 0;
                }

                sb += (end->b * end->a) - (begin->b * begin->a);
                sg += (end->g * end->a) - (begin->g * begin->a);
                sr += (end->r * end->a) - (begin->r * begin->a);
                sa += end->a - begin->a;
            }
        }
    }

private:
    void AccumulateFirstKernal(const RGBA32bit* px, float& sr, float& sg, float& sb, float& sa) const
    {
        if (m_only_blur_alpha)
        {
            for (int i = 0; i < m_kernal_length; i++, px++)
            {
                sa += px->a;
            }
        }
        else if (m_all_alpha_0xFF && m_copy_edge_pixel)
        {
            for (int i = 0; i < m_kernal_length; i++, px++)
            {
                sb += px->b;
                sg += px->g;
                sr += px->r;
            }
        }
        else
        {
            for (int i = 0; i < m_kernal_length; i++, px++)
            {
                sb += (px->b * px->a);
                sg += (px->g * px->a);
                sr += (px->r * px->a);
                sa += px->a;
            }
        }
    }

    void FillPadding(RGBA32bit*& curr, const void* edge_pixel) const
    {
        RGBA32bit   padding = (m_copy_edge_pixel ? *(RGBA32bit*)edge_pixel : RGBA32bit{ 0,0,0,0 });
        for (int i = 0; i < m_radius; i++)
            *curr++ = padding;
    }
};

class FCEffectBoxBlurVert : public FCImageEffect
{
private:
    FCBoxBlurAccumulator   m_acc;
public:
    FCEffectBoxBlurVert(const FCBoxBlurAccumulator& acc) : m_acc(acc) {}
private:
    virtual SIZE QueryScanLineCountEachParallelTask(const FCImage& img) { return CSize(200, 0); }
    virtual bool IsSupport(const FCImage& img) { return true; }
    virtual ProcessMode QueryProcessMode() { return ProcessMode::Region; }

    virtual void ProcessRegion(FCImage& img, CRect rc, FCProgressObserver* progress)
    {
        // because this function may be called from other thread, it must have its own scan line buffer.
        auto   line_buffer = m_acc.CreateLineBuffer(img.Height());
        auto   ptr = line_buffer.data();

        for (int x = rc.left; x < rc.right; x++)
        {
            m_acc.VerticalGetLine(img, x, ptr);
            m_acc.BlurScanLine((RGBA32bit*)img.GetBits(x, 0), img.Width(), ptr, img.Height());
            if (progress && !progress->UpdateProgress(0)) // need call it for cancel
                break;
        }
    }
};

class FCEffectBoxBlurHoriz : public FCImageEffect
{
private:
    FCBoxBlurAccumulator   m_acc;
public:
    FCEffectBoxBlurHoriz(const FCBoxBlurAccumulator& acc) : m_acc(acc) {}
private:
    virtual SIZE QueryScanLineCountEachParallelTask(const FCImage& img) { return CSize(0, 200); }
    virtual bool IsSupport(const FCImage& img) { return true; }
    virtual ProcessMode QueryProcessMode() { return ProcessMode::Region; }

    virtual void ProcessRegion(FCImage& img, CRect rc, FCProgressObserver* progress)
    {
        // because this function may be called from other thread, it must have its own scan line buffer.
        auto   line_buffer = m_acc.CreateLineBuffer(img.Width());
        auto   ptr = line_buffer.data();

        for (int y = rc.top; y < rc.bottom; y++)
        {
            m_acc.HorizontalGetLine(img, y, ptr);
            m_acc.BlurScanLine((RGBA32bit*)img.GetBits(y), 1, ptr, img.Width());
            if (progress && !progress->UpdateProgress(0))
                break;
        }
    }
};
_IMSTONE_PRIVATE_END

// Based on an article of Ivan Kuckir: http://blog.ivank.net/fastest-gaussian-blur.html
/// Gaussian blur (32 bit).
class FCEffectGaussianBlur : public FCImageEffect
{
private:
    int   m_radius;
    bool   m_copy_edge_pixel;
    bool   m_only_blur_alpha;

public:
    FCEffectGaussianBlur(int radius, bool copy_edge_pixel, bool only_blur_alpha = false)
    {
        m_radius = std::clamp(radius, 1, 500);
        m_copy_edge_pixel = copy_edge_pixel;
        m_only_blur_alpha = only_blur_alpha;
    }

private:
    virtual bool IsSupport(const FCImage& img)
    {
        return (img.ColorBits() == 32) && !img.IsPremultiplied();
    }

    virtual ProcessMode QueryProcessMode()
    {
        return ProcessMode::EntireMyself;
    }

    static auto boxesForGauss(double sigma, int n) // standard deviation, number of boxes
    {
        double   wIdeal = sqrt((12 * sigma * sigma / n) + 1); // Ideal averaging filter width 
        int   wl = (int)floor(wIdeal);
        if (wl % 2 == 0) { wl--; }
        int   wu = wl + 2;
        double   mIdeal = (12 * sigma*sigma - n * wl*wl - 4 * n*wl - 3 * n) / (-4 * wl - 4);
        int   m = (int)round(mIdeal);

        std::vector<int>   ret;
        for (int i = 0; i < n; i++) { ret.push_back(i < m ? wl : wu); }
        return ret;
    }

    auto CreateBoxBlurGroup(const FCImage& img) const
    {
        bool   all_alpha_0xFF = FCImageHandle::IsAllPixelAlpha0xFF(img);
        auto   sub_boxes = boxesForGauss(m_radius, 3);

        std::vector<std::unique_ptr<FCImageEffect>>   ret;
        for (auto iter : sub_boxes)
        {
            int   r = (iter - 1) / 2;
            if (r >= 1)
            {
                imsprivate::FCBoxBlurAccumulator   acc(r, m_copy_edge_pixel, m_only_blur_alpha, all_alpha_0xFF);
                ret.push_back(std::make_unique<imsprivate::FCEffectBoxBlurVert>(acc));
                ret.push_back(std::make_unique<imsprivate::FCEffectBoxBlurHoriz>(acc));
            }
        }
        return ret;
    }

    virtual void ProcessEntire(FCImage& img, FCProgressObserver* progress)
    {
        if (progress) { progress->BeginFixProgress(0); }

        auto   effect_group = CreateBoxBlurGroup(img);
        for (auto& eff : effect_group)
        {
            eff->EnableParallelAccelerate(IsParallelAccelerateEnable());
            img.ApplyEffect(*eff, progress);
        }

        if (progress) { progress->EndFixProgress(); }
    }
};
