char igeq(int a, int b) {
   return a >= b;
}

char bgeq(char a, char b) {
   return a >= b;
}

char geq(int a, int b) {
   return igeq(a, b) && bgeq(a, b);
}
