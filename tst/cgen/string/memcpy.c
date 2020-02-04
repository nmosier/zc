typedef long size_t;

void *mymemcpy(void *dst, void *src, size_t n) {
   char *dst_it, *src_it;
   
   for (dst_it = dst, src_it = src; n > 0; --n) {
      *dst_it++ = *src_it++;
   }
   
   return dst;
}
