#pragma once

namespace imagestone_inner
{

struct CGdiplusSaveParam
{
    CLSID   m_type_CLSID;
    ULONG   m_jpeg_quality;
    std::unique_ptr<Gdiplus::EncoderParameters>   m_encoder_param;

    CGdiplusSaveParam(PCWSTR image_path, int jpeg_quality)
    {
        auto   image_type = CImageFileExtParser::GetType(image_path);
        m_type_CLSID = GetEncoderClsid(image_type);
        m_jpeg_quality = jpeg_quality;

        if ((image_type == ImageFormat::Jpeg) && jpeg_quality)
        {
            m_encoder_param.reset(new Gdiplus::EncoderParameters);
            m_encoder_param->Count = 1;
            m_encoder_param->Parameter[0].Guid = Gdiplus::EncoderQuality;
            m_encoder_param->Parameter[0].Type = Gdiplus::EncoderParameterValueTypeLong;
            m_encoder_param->Parameter[0].NumberOfValues = 1;
            m_encoder_param->Parameter[0].Value = &m_jpeg_quality;
        }
    }

private:
    static GUID GetFormatGUID(ImageFormat img_type)
    {
        GUID   fmt = { 0 };
        switch (img_type)
        {
        case ImageFormat::Bmp: fmt = Gdiplus::ImageFormatBMP; break;
        case ImageFormat::Jpeg: fmt = Gdiplus::ImageFormatJPEG; break;
        case ImageFormat::Gif: fmt = Gdiplus::ImageFormatGIF; break;
        case ImageFormat::Tiff: fmt = Gdiplus::ImageFormatTIFF; break;
        case ImageFormat::Png: fmt = Gdiplus::ImageFormatPNG; break;
        }
        return fmt;
    }

    static CLSID GetEncoderClsid(ImageFormat img_type)
    {
        CLSID   ret = { 0 };
        UINT   num = 0, buf_size = 0;
        Gdiplus::GetImageEncodersSize(&num, &buf_size);
        if (num && buf_size)
        {
            std::vector<BYTE>   temp_buf(buf_size);
            auto   info = (Gdiplus::ImageCodecInfo*)temp_buf.data();
            Gdiplus::GetImageEncoders(num, buf_size, info);

            GUID   fmtid = GetFormatGUID(img_type);
            for (UINT i = 0; i < num; i++)
            {
                if (info[i].FormatID == fmtid)
                {
                    ret = info[i].Clsid;
                    break;
                }
            }
        }
        return ret;
    }
};

}
