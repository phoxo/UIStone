#pragma once
#include "render_target_creator.h"

class D2DSVGDrawer
{
private:
    IStreamPtr   m_stream;
    ID2D1FactoryPtr   m_factory;

public:
    D2DSVGDrawer(IStream* svg_stream, ID2D1Factory* factory)
    {
        m_stream = svg_stream;
        m_factory = factory;
    }

    // ע�⣺�����ʽ�� premultiplied alpha
    IWICBitmapPtr CreateBitmap(SIZE output_size, float scale)
    {
        // from Windows 10 1703, build 15063. �汾̫�Ϸ���NULL
        auto   bmp = CWICFunc::CreateBitmap(output_size, WICPremultiplied32bpp); assert(bmp);
        ID2D1DeviceContext5Ptr   dc5 = D2DRenderTargetCreator::CreateWicBitmapRenderTarget(m_factory, bmp); // <== ����ʽת��
        if (dc5 && m_stream)
        {
            if (scale != 1.0f)
            {
                dc5->SetTransform(D2D1::Matrix3x2F::Scale(scale, scale));
            }

            if (DrawSvgDocument(dc5, output_size))
                return bmp;
        }
        return nullptr;
    }

private:
    bool DrawSvgDocument(ID2D1DeviceContext5* dc, SIZE output_size)
    {
        ID2D1SvgDocumentPtr   svg;
        dc->CreateSvgDocument(m_stream, CD2DSizeF(output_size), &svg);
        if (svg)
        {
            dc->BeginDraw();
            dc->DrawSvgDocument(svg);
            dc->EndDraw();
            return true;
        }
        return false;
    }
};
