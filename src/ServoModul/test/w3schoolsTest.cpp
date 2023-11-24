#include <iostream>
using namespace std;

int main() {
  cout << "Hello World!";
  bool target = false;
  bool currentStateMinus = true;
  bool currentStatePlus = true;
  int8_t currentSwitchState = 2 - currentStateMinus - currentStatePlus*2;
  if(!target = currentSwitchState) {
  	cout << "ja";
  }
  return 0;
}
