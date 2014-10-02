#include <string>
#include <sstream>
#include <iostream>

#include <windows.h>
#include <shlobj.h>

using namespace std;

string GetUserHomeDirectory() {
  char localAppData[MAX_PATH];

  if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, localAppData)))
    return string(localAppData);
  else
    return ".";
}
