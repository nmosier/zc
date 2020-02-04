typedef char bool;

bool isupper(char c) {
   return c >= 'A' && c <= 'Z';
}

char toupper(char c) {
   if (c >= 'a' && c <= 'z') {
      return c + ('A' - 'a');
   } else {
      return c;
   }
}

int main(int argc, char **argv) {
   return isupper(toupper(argc));
}
