#ifndef JGEOMETRY_UTIL_H
#define JGEOMETRY_UTIL_H

#include <math.h>
#include <JSystem/JMath/JMath.h>
#include "types.h"

namespace JGeometry {
    template <typename T>
    class TUtil
    {
    public:
        static f32 abs(f32 x) { return __fabsf(x); }
        static f32 atan2(f32 y, f32 x) { return JMAAtan2Radian(y, x); }
        static const f32 epsilon() { return 32.0f * FLT_EPSILON; }

        static bool epsilonEquals(T a1, T a2, T a3)
        {
            bool equal = false;

            a1 -= a2;

            if (-a3 <= a1 && a1 <= a3)
            {
                equal = true;
            }

            return equal;
        }

        f32 invert(f32 x) { return 1.0f/x; }

        static f32 sqrt(f32 x)
        {
            if (x <= 0.0f)
                return x;

            f32 y = __frsqrte(x);
            y = 0.5f * y * (3.0f - (x * (y * y)));
            return x * y;
        }

        static f32 inv_sqrt(f32 x)
        {
            if (x <= 0.0f)
                return x;
            
            f32 y = __frsqrte(x);
            y = 0.5f * y * (3.0f - (x * (y * y)));
            return y;
        }

        T one() { return (T)1; }

        // TODO: probably uses seomthing from TSinCosTable
        /*static f32 asin(f32 val) NO_INLINE
        {
            if (val >= 1.0f)
            {
                return 1.5707964f;
            }

            if (val <= -1.0f)
            {
                return -1.5707964f;
            }

            if (val < 0.0f)
            {
                return -flt_80630CA0[(u32)(1023.5f * -val)];
            }

            return flt_80630CA0[(u32)(1023.5f * val)];
        }*/

        /*static f32 PI() NO_INLINE
        {
            return 3.1415927f;
        }*/

        static T clamp(T p1, T p2, T p3);
    };

    typedef TUtil<f32> TUtilf;
}


#endif
