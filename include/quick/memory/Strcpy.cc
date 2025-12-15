char *Strcpy(char *dst, const char *src)
{

    char *result = dst;
    while ((*dst++ = *src++))
        ;
    return result;
}