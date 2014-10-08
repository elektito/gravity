#include <string>
#include <iostream>

#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>

using namespace std;

string GetUserHomeDirectory() {
  string dir = getenv("HOME");
  if (dir.empty()) {
    struct passwd* pwd = getpwuid(getuid());
    dir = pwd->pw_dir;
  }

  return dir;
}

void ShowMessage(string msg) {
  cout << msg << endl;
}
