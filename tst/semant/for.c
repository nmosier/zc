int main(int argc, char **argv) {
   int i, j;

   for (i = 0, j = 0; i < 10; ++i) {
      j = j + argc;
   }
   
   return j;
}
