int iland(int a, int b) {
   return a && b;
}

int bland(char a, char b) {
   return a && b;
}

int land(int a, int b) {
   return iland(a, b) && bland(a, b);
}
