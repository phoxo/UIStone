#pragma once

/// Helper class
class FCMath
{
public:
    static inline BYTE Clamp0255(int n)
    {
        return (BYTE)std::clamp(n, 0, 0xFF);
    }

    /// if d is very large, Clamp0255((int)d) will get result 0, so we must clamp to [0,255] first, then convert to int.
    static inline BYTE Clamp0255(const double& d)
    {
        return (BYTE)(std::clamp(d, 0.0, 255.0) + 0.5);
    }

    static int Rand()
    {
        return (((m_rand_hold = m_rand_hold * 214013L + 2531011L) >> 16) & 0x7fff); // RAND_MAX
    }

    static inline constexpr float   LIB_PI = 3.14159274f;

private:
    static inline long   m_rand_hold = (long)::GetTickCount();
};
