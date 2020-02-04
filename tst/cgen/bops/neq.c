char ineq(int a, int b) {
   return a != b;
}

char bneq(char a, char b) {
   return a != b;
}

char neq(int a, int b) {
   return ineq(a, b) && bneq(a, b);
}
