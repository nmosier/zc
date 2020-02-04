int ilor(int a, int b) {
   return a || b;
}

int blor(char a, char b) {
   return a || b;
}

int lor(int a, int b) {
   return ilor(a, b) || blor(a, b);
}
