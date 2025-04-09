#pragma once
#include "sequence_image.h"

// 根据一张图片制作 normal - hover - pressed 图片
class Sequence3StateBuilder
{
private:
    // Note: 会直接修改 src
    static void CoverState(auto& output, auto& src, auto* effect, int index)
    {
        src.ApplyEffectAndDelete(effect);
        ImageHandler::Cover(output, src, CPoint(src.Width() * index, 0));
    }

public:
    // 返回的 image 是 PremultipliedAlpha
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
            CoverState(out, src, new eff::BrightnessContrast(-20, -3), 2); // pressed，Note: 直接修改src，反正之后不用了
            out.ApplyEffectAndDelete(new eff::Premultiply);
        }
        return out;
    }
};
