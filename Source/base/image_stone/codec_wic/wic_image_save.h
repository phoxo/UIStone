#pragma once
#include "wic_metadata.h"

class CWICFileEncoder
{
private:
    GUID   m_image_format = GUID_NULL;
    IWICBitmapEncoderPtr   m_encoder;
    IWICStreamPtr   m_stream;
    IWICBitmapFrameEncodePtr   m_frame_encode;

public:
    CWICFileEncoder(PCWSTR image_path, int jpeg_quality = 80)
    {
        m_image_format = CWICInstalledCodec::FindContainerFormat(image_path);
        HRESULT   hr = CWICFunc::g_factory->CreateEncoder(m_image_format, NULL, &m_encoder);
        if (!m_encoder)
        {
            assert(hr == WINCODEC_ERR_COMPONENTNOTFOUND); // ��֧�ֵ�ͼ������ʽ
            return;
        }

        try
        {
            CWICFunc::g_factory->CreateStream(&m_stream);
            if (m_stream->InitializeFromFilename(image_path, GENERIC_WRITE) == S_OK) // ����ļ�д������ʧ��
            {
                m_encoder->Initialize(m_stream, WICBitmapEncoderNoCache);
                CreateFrameEncode(jpeg_quality);
            }
        }
        catch (_com_error&) {}
        assert(IsEncoderAvailable());
    }

    bool IsEncoderAvailable() const
    {
        return (m_frame_encode != NULL);
    }

    bool IsJPEG() const { return (m_image_format == GUID_ContainerFormatJpeg); }

/*    void CopyMetadata(IWICBitmapFrameDecodePtr source_meta)
    {
        IWICMetadataBlockReaderPtr   reader = source_meta;
        IWICMetadataBlockWriterPtr   writer = m_frame_encode;
        if (writer && reader)
        {
            writer->InitializeFromBlockReader(reader);

            IWICMetadataQueryWriterPtr   orient;
            m_frame_encode->GetMetadataQueryWriter(&orient);
            CWICMetadataOrientation::Write(orient, 1); // clear orientation tag
        }
    }*/

    bool SetICC(IWICColorContext* icc)
    {
        if (m_frame_encode && icc && IsICCSaveSupported(icc))
        {
            return (m_frame_encode->SetColorContexts(1, &icc) == S_OK);
        }
        return false;
    }

    bool WriteFile(IWICBitmapSourcePtr src)
    {
        bool   t[3] = {};
        try
        {
            t[0] = (m_frame_encode->WriteSource(src, NULL) == S_OK);
            t[1] = (m_frame_encode->Commit() == S_OK);
            t[2] = (m_encoder->Commit() == S_OK);
        }
        catch (_com_error&) { assert(false); }
        return (t[0] && t[1] && t[2]);
    }

private:
    bool IsICCSaveSupported(IWICColorContext* icc) const
    {
        if (icc)
        {
            if (IsJPEG()) // jpeg֧����������icc
                return true;

            if ((m_image_format == GUID_ContainerFormatPng) || (m_image_format == GUID_ContainerFormatTiff))
            {
                // png��֧��exif���ͣ�����������WriteSource��ʧ�ܣ�tiff֧��exif���ͣ���ȫ����Ȳ�֧����
                UINT   len = 0;
                icc->GetProfileBytes(0, NULL, &len);
                return (len != 0);
            }
        }
        return false;
    }

    bool SetOrientationTag(int orientation)
    {
        if (m_frame_encode)
        {
            IWICMetadataQueryWriterPtr   writer;
            m_frame_encode->GetMetadataQueryWriter(&writer);
            return CWICMetadataOrientation::Write(writer, orientation);
        }
        return false;
    }

    void CreateFrameEncode(int jpeg_quality)
    {
        try
        {
            if (IsJPEG())
            {
                IPropertyBag2Ptr   prop;
                m_encoder->CreateNewFrame(&m_frame_encode, &prop);
                WriteImageQualityProperty(prop, jpeg_quality / 100.0f);
                m_frame_encode->Initialize(prop);
                SetOrientationTag(1); // ���û�з���tag���Ժ������תjpg��ʧ��
            }
            else
            {
                m_encoder->CreateNewFrame(&m_frame_encode, nullptr);
                m_frame_encode->Initialize(nullptr);
            }
        }
        catch (_com_error&) { assert(false); }
    }

    static void WriteImageQualityProperty(IPropertyBag2* prop, float quality)
    {
        _variant_t   val(quality);
        _bstr_t   prop_name = L"ImageQuality";
        PROPBAG2   str = {};
        str.pstrName = prop_name;
        if (prop) { prop->Write(1, &str, &val); }
    }
};
