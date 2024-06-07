#pragma once

// forward declaration
class FCImage;

/// Image effect processor interface.
class FCImageEffect
{
private:
    bool   m_parallel_accelerate = false;

public:
    /// process mode.
    enum class ProcessMode
    {
        PixelByPixel,
        Region,
        EntireMyself,
    };

public:
    virtual ~FCImageEffect() {}

    /// @name Parallel switch.
    ///@{
    bool IsParallelAccelerateEnable() const { return m_parallel_accelerate; }
    void EnableParallelAccelerate(bool enable) { m_parallel_accelerate = enable; }
    virtual SIZE QueryScanLineCountEachParallelTask(const FCImage& img);
    /// return std::thread::hardware_concurrency() by default.
    virtual int QueryMaxParallelCount()
    {
        static int   v = GetLogicalProcessorsNumber();
        return std::clamp(v, 8, 48);
    }
    ///@}

    /// check bpp >= 24 by default.
    virtual bool IsSupport(const FCImage& img);
    /// default return <b>PixelByPixel</b>.
    virtual ProcessMode QueryProcessMode() { return ProcessMode::PixelByPixel; }

    /// event before process, default do nothing.
    virtual void OnBeforeProcess(FCImage& img) {}
    virtual void OnAfterProcess(FCImage& img) {}

    /// process (x,y) pixel when \ref QueryProcessMode return <b>PixelByPixel</b>.
    virtual void ProcessPixel(FCImage& img, int x, int y, RGBA32bit* pixel) { assert(false); }
    /// process portion of an image when \ref QueryProcessMode return <b>Region</b>.
    virtual void ProcessRegion(FCImage& img, CRect region_rect, FCProgressObserver* progress) { assert(false); }
    /// process the entire image when \ref QueryProcessMode return <b>EntireMyself</b>.
    virtual void ProcessEntire(FCImage& img, FCProgressObserver* progress) { assert(false); }

private:
    static int GetLogicalProcessorsNumber()
    {
        SYSTEM_INFO   v = { 0 };
        ::GetNativeSystemInfo(&v);
        return v.dwNumberOfProcessors;
    }
};
