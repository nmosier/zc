char ilt(int a, int b) {
   return a < b;
}

char blt(char a, char b) {
   return a < b;
}

char lt(int a, int b) {
   return ilt(a, b) && blt(a, b);
}
