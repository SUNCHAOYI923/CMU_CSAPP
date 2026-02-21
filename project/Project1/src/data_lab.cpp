#include "../include/data_lab.hpp"
#include <cstdint>

namespace data_lab
{
    int32_t add (int32_t a, int32_t b)
    {
        int32_t p = a ^ b, g = a & b;
        g |= (p & (g << 1));p &= (p << 1);
        g |= (p & (g << 2));p &= (p << 2);
        g |= (p & (g << 4));p &= (p << 4);
        g |= (p & (g << 8));p &= (p << 8);
        g |= (p & (g << 16)); 
        return (a ^ b) ^ (g << 1);
    }

    int32_t subtract (int32_t a, int32_t b)
    {
        return add (a, add (~b, 1));
    }

    int32_t multiply (int32_t a, int32_t b)
    {
        int32_t sign = 0;
        uint32_t tmpa = static_cast <uint32_t> (a);
        uint32_t tmpb = static_cast <uint32_t> (b);
        uint32_t tmpres = 0;
        if (tmpa >> 31 & 1) 
        {
            tmpa = subtract (0, tmpa);
            sign ^= 1;
        }
        if (tmpb >> 31 & 1)
        {
            tmpb = subtract (0, tmpb);
            sign ^= 1;
        }
        while (tmpb)
        {
            if (tmpb & 1) tmpres = add (tmpres, tmpa);
            tmpa <<= 1;tmpb >>= 1;
        }
        int32_t res = static_cast <int32_t> (tmpres);
        if (sign) res = subtract (0, res);
        return res;
    }

    int32_t divide (int32_t a, int32_t b)
    {
        int32_t sign = 0;
        uint32_t tmpa = static_cast <uint32_t> (a);
        uint32_t tmpb = static_cast <uint32_t> (b);
        uint32_t tmpres = 0;
        if (tmpa >> 31 & 1) 
        {
            tmpa = subtract (0, tmpa);
            sign ^= 1;
        }
        if (tmpb >> 31 & 1)
        {
            tmpb = subtract (0, tmpb);
            sign ^= 1;
        }
        for (int i = 31;~i;--i)
        {
            int32_t diff = subtract (tmpa >> i, tmpb);
            if (!(diff >> 31 & 1))
            {
                tmpres |= (1 << i);
                tmpa = subtract (tmpa, tmpb << i);
            }
        }
        int32_t res = static_cast <int32_t> (tmpres);
        if (sign) res = subtract (0, res);
        return res;
    }

    int32_t modulo (int32_t a, int32_t b)
    {
        int32_t q = divide (a, b);
        int32_t mx = multiply (b, q);
        return subtract (a, mx);
    }
}