#pragma once

// image format
enum class ImageFormat
{
    Unknown,
    Bmp,
    Jpeg,
    Gif,
    Tiff,
    Png,
    Icon,
    Psd,
    Tga,
    Webp,
    Raw,
    Dds,
    Dng,
    Svg,
    Heif,
    Avif,
    Freeimage,
};

class CImageFileExtParser
{
public:
    /// get image's format by file's ext name.
    static ImageFormat GetType(PCWSTR file_name)
    {
        CString   ext = PathFindExtension(file_name);
        if (ext.GetLength())
            ext.Delete(0, 1);
        ext = L"," + ext + L",";

        if (StrStrI(ImageExtJpeg(), ext))  return ImageFormat::Jpeg;
        if (StrStrI(L",png,", ext))  return ImageFormat::Png;
        if (StrStrI(L",bmp,dib,", ext))  return ImageFormat::Bmp;
        if (StrStrI(L",gif,", ext))  return ImageFormat::Gif;
        if (StrStrI(L",tiff,tif,", ext))  return ImageFormat::Tiff;
        if (StrStrI(L",ico,icon,", ext))  return ImageFormat::Icon;
        if (StrStrI(L",psd,", ext))  return ImageFormat::Psd;
        if (StrStrI(L",tga,", ext))  return ImageFormat::Tga;
        if (StrStrI(L",webp,", ext))  return ImageFormat::Webp;
        if (StrStrI(ImageExtRaw(), ext))  return ImageFormat::Raw;
        if (StrStrI(L",dds,", ext))  return ImageFormat::Dds;
        if (StrStrI(L",dng,", ext))  return ImageFormat::Dng;
        if (StrStrI(L",svg,", ext))  return ImageFormat::Svg;
        if (StrStrI(L",heif,heic,", ext))  return ImageFormat::Heif;
        if (StrStrI(L",avif,", ext))  return ImageFormat::Avif;
        if (StrStrI(ImageExtFreeimage(), ext))  return ImageFormat::Freeimage;
        return ImageFormat::Unknown;
    }

public:
    static PCWSTR ImageExtJpeg() { return L",jpg,jpeg,jfif,jpe,"; }
    static PCWSTR ImageExtRaw() { return L",3fr,ari,arw,bay,cap,cr2,cr3,crw,dcs,dcr,drf,eip,erf,fff,iiq,k25,kdc,mef,mos,mrw,nef,nrw,orf,ori,pef,ptx,pxn,raf,raw,rw2,rwl,sr2,srf,srw,x3f,dng,"; }
    static PCWSTR ImageExtFreeimage() { return L",exr,g3,hdr,iff,lbm,j2k,j2c,jp2,jxr,wdp,hdp,pcd,pcx,pfm,pct,pict,pic,pbm,pgm,ppm,ras,sgi,rgb,rgba,bw,wap,wbmp,wbm,xbm,xpm,"; }
};
