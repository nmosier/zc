typedef long size_t;

void *memset(void *buf, char c, size_t len) {
   char *buf_it;

   for (buf_it = buf; len > 0; --len) {
      *buf_it++ = c;
   }

   return buf;
}

void *memcpy(void *dst, void *src, size_t n) {
   char *dst_it, *src_it;
   
   for (dst_it = dst, src_it = src; n > 0; --n) {
      *dst_it++ = *src++;
   }
   
   return dst;
}

int main(int argc, char **argv) {
   char dst[10];
   char src[10];

   memset(src, 42, 10);
   memcpy(dst, src, 10);

   return 0;
}
