int main(int argc, char **argv) {
   while (1) {
      if (argc <= 10) {
         break;
      }
      argc = argc - 1;
   }

   return argc;
}
