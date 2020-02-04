char igt(int a, int b) {
   return a > b;
}

char bgt(char a, char b) {
   return a > b;
}

char gt(int a, int b) {
   return igt(a, b) && bgt(a, b);
}
