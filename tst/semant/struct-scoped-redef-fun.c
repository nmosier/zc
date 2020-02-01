struct coords {
   int x;
   int y;
};

int main(int argc, char **argv) {
   struct coords *ptr;
   struct coords {
      int a;
      int b;
   } pt;

   ptr = &pt;

   return 0;
}
