#pragma once
#include "sequence_image.h"

// ����һ��ͼƬ���� normal - hover - pressed ͼƬ
class Sequence3StateBuilder
{
private:
    // Note: ��ֱ���޸� src
    static void CoverState(auto& output, auto& src, auto* effect, int index)
    {
        src.ApplyEffectAndDelete(effect);
        ImageHandler::Cover(output, src, CPoint(src.Width() * index, 0));
    }

public:
    // ���ص� image �� PremultipliedAlpha
    static phoxo::Image Load(PCWSTR filepath, int image_designed_for_dpi = 2 * USER_DEFAULT_SCREEN_DPI)
    {
        namespace eff = phoxo::effect;

        SequenceImage   loader(filepath, image_designed_for_dpi, 1);
        phoxo::Image   out;
        if (auto src = loader.Load(WICNormal32bpp))
        {
            out.Create(src.Width() * 3, src.Height(), 32);
            ImageHandler::Cover(out, src, CPoint(0, 0)); // normal
            auto   hover = src;
            CoverState(out, hover, new eff::BrightnessContrast(25, 3), 1); // hover
            CoverState(out, src, new eff::BrightnessContrast(-20, -3), 2); // pressed��Note: ֱ���޸�src������֮������
            out.ApplyEffectAndDelete(new eff::Premultiply);
        }
        return out;
    }
};
