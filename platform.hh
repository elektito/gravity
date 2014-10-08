#include <string>

using namespace std;

#define SHOW_MSG(msg) { stringstream ss; ss << msg; ShowMessage(ss.str()); }

extern string GetUserHomeDirectory();
extern void ShowMessage(string msg);
