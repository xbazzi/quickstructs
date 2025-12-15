void *Memmove(void *dst, const void *src, std::size_t n)
{
    if (!src or !dst or n == 0)
        return dst;
    unsigned char *to = static_cast<unsigned char *>(dst);
    const unsigned char *from = static_cast<const unsigned char *>(src);
    if (to < from)
    {
        while (n--)
        {
            *to++ = *from++;
        }
    }
    else if (from < to)
    {
        to += n;
        from += n;
        while (n--)
        {
            *--to = *--from;
        }
    }
    return dst;
}