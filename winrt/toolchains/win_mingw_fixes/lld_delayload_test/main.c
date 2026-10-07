#include <windows.h>
#include <stdio.h>
#include <shlwapi.h>
int main(void){
  /* PathIsRelativeA/StrCmpLogicalW from shlwapi.dll, delay-loaded -> first call goes
     through __tailMerge_shlwapi_dll -> __delayLoadHelper2. */
  BOOL r = PathIsRelativeA("foo\\bar");
  int c = StrCmpLogicalW(L"a2", L"a10");
  HMODULE h = GetModuleHandleA("shlwapi.dll");
  printf("PathIsRelativeA=%d StrCmpLogicalW=%d shlwapi_loaded=%d\n", r, c, h != NULL);
  puts((r == 1 && c < 0 && h) ? "DELAYLOAD_PASS" : "DELAYLOAD_FAIL");
  return 0;
}
