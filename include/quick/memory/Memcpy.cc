#include <cstdint>

void *Memcpy(void *dst, const void *src, std::size_t n)
{
    unsigned char *to = static_cast<unsigned char *>(dst);
    const unsigned char *from = static_cast<const unsigned char *>(src);
    while (n >= 8)
    {
        *(to + 0) = *(from + 0);
        *(to + 1) = *(from + 1);
        *(to + 2) = *(from + 2);
        *(to + 3) = *(from + 3);
        *(to + 4) = *(from + 4);
        *(to + 5) = *(from + 5);
        *(to + 6) = *(from + 6);
        *(to + 7) = *(from + 7);
        to += 8;
        from += 8;
        n -= 8;
    }

    while (n--)
    {
        *to++ = *from++;
    }
    return dst;
}