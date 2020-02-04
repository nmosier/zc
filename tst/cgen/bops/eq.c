char ieq(int a, int b) {
   return a == b;
}

char beq(char a, char b) {
   return a == b;
}

char eq(int a, int b) {
   return ieq(a, b) && beq(a, b);
}
