typedef long size_t;

void *mymemset(void *buf, char c, size_t len) {
   char *buf_it;

   for (buf_it = buf; len > 0; --len) {
      *buf_it++ = c;
   }

   return buf;
}
