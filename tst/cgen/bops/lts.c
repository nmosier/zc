char ilts(int a, int b) {
   return a < b;
}

char blts(char a, char b) {
   return a < b;
}

char lts(int a, int b) {
   return ilts(a, b) && blts(a, b);
}
