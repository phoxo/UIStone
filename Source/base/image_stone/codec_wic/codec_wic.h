#pragma once
#include "wic_image_save.h"

class FCCodecWIC
{
public:
    static bool LoadFile(PCWSTR image_path, FCImage& img, REFWICPixelFormatGUID output_format = WICNormal32bpp, CWICMetadata* meta = NULL, BOOL use_embedded_icc = FALSE)
    {
        auto   stm = CWICFunc::CreateStreamFromFileNoLock(image_path);
        return LoadStream(stm, img, output_format, meta, use_embedded_icc);
    }

    static bool LoadStream(IStream* stm, FCImage& img, REFWICPixelFormatGUID output_format, CWICMetadata* meta = NULL, BOOL use_embedded_icc = FALSE)
    {
        auto   decoder = CWICFunc::CreateDecoderFromStream(stm);
        auto   first_frame = CWICFunc::GetFrame(decoder, 0); // just load first frame

        if (meta)
        {
            meta->Read(first_frame);
        }
        return LoadFrame(first_frame, img, output_format, use_embedded_icc);
    }

    static bool LoadFrame(IWICBitmapFrameDecode* frame_decode, FCImage& img, REFWICPixelFormatGUID output_format, BOOL use_embedded_icc = FALSE)
    {
        // �ȸı��ʽ����Ҫ
        // 1. ������һ�θ�ʽ��һ��apply ICCʱ����
        // 2. ��JPG, ���32bpp�����Է��ֶ���Ҫ��ת��jpg��ת��32bpp�ٶȸ���
        // 3. ת��Ŀ���ʽ���Һ������load��ʽһ�£�������ת��ʱ����Ҫʵ�廯
        IWICBitmapSourcePtr   dest = CWICFunc::ConvertFormat(frame_decode, output_format); // <-- ��ʽһ��

        if (use_embedded_icc)
        {
            auto   icc = CWICFunc::GetFirstColorContext(frame_decode);
            if (icc)
            {
                dest = ApplyEmbeddedICC(dest, icc);
            }
        }

        auto   rotate = CWICMetadataOrientation::Read(frame_decode);
        if (rotate != WICBitmapTransformRotate0)
        {
            dest = CorrectOrientation(dest, rotate);
        }

        return Load(dest, img, output_format); // <-- ��ʽһ��
    }

    static bool Load(IWICBitmapSource* src_bmp, FCImage& img, WICPixelFormatGUID output_format = GUID_WICPixelFormatDontCare)
    {
        if (output_format == GUID_WICPixelFormatDontCare)
            output_format = CWICFunc::GetPixelFormat(src_bmp);

        int   bpp = CWICFunc::GetBitsPerPixel(output_format);
        int   attr = ((output_format == WICPremultiplied32bpp) ? FCImage::PremultipliedAlpha : 0);
        auto   src = CWICFunc::ConvertFormat(src_bmp, output_format);
        if (src)
        {
            CSize   sz = CWICFunc::GetBitmapSize(src);
            if (img.Create(sz.cx, sz.cy, bpp, attr))
            {
                if (src->CopyPixels(NULL, img.GetStride(), img.GetPixelBufferSize(), (BYTE*)img.GetMemStart()) == S_OK)
                    return true;
            }
        }
        assert(false);
        img.Destroy();
        return false;
    }

    static IWICBitmapSourcePtr ApplyEmbeddedICC(IWICBitmapSourcePtr src_bmp, IWICColorContextPtr src_icc, BOOL restore_from_srgb = FALSE)
    {
        auto   dest_icc = CWICFunc::CreateSystemColorContext_SRGB();
        if (restore_from_srgb)
        {
            std::swap(src_icc, dest_icc);
        }

        // ��Ҫ�ı�icc���ظ�ʽ����һ�ν���.cr2��ʽ��������ʱ��
        auto   format = CWICFunc::GetPixelFormat(src_bmp);
        auto   trans = CWICFunc::CreateColorTransformer();
        if (trans)
        {
            if (trans->Initialize(src_bmp, src_icc, dest_icc, format) == S_OK)
                return trans;
        }
        // assert(false); �ص�assert��һЩgoogle sRGBҲת��ʧ��
        return src_bmp;
    }

private:
    static IWICBitmapSourcePtr CorrectOrientation(IWICBitmapSourcePtr src, WICBitmapTransformOptions flag)
    {
        auto   cmd = CWICFunc::CreateBitmapFlipRotator();
        if (cmd)
        {
            if (cmd->Initialize(src, flag) == S_OK)
                return cmd;
        }
        assert(false);
        return src;
    }
};
