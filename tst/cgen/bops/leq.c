char ileq(int a, int b) {
   return a <= b;
}

char bleq(char a, char b) {
   return a <= b;
}

char leq(int a, int b) {
   return ileq(a, b) && bleq(a, b);
}
