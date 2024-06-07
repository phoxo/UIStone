#pragma once

inline void FCImage::ApplyEffectParallel(FCImageEffect& effect, FCProgressObserver* progress)
{
    imagestone_inner::FCParallelEffectTaskExecutor   exec(*this, effect);
    exec.ParallelExecuteTask(progress);
}

inline bool FCImageEffect::IsSupport(const FCImage& img)
{
    return (img.ColorBits() >= 24);
}

inline SIZE FCImageEffect::QueryScanLineCountEachParallelTask(const FCImage& img)
{
    CSize   ret(0, 200);
    return ret;
}
