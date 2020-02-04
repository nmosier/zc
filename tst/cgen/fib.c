int main(int i) {
   if (i <= 1) {
      return i;
   } else {
      return main(i - 1)  + main(i - 2);
   }
}
