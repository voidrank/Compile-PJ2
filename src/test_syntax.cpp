#include <iostream>
#include "syntax.h"

int main () {
  Body* b = new Body();
  Program* p = new Program(b);
 
  Node* n = b;

  p->print(0);
  return 0;
}
