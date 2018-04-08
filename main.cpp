#include <cstdio>
#include <windows.h>
using namespace std;

void usage() {
    static const char *name = "rcopy";
    fprintf(stderr, "Copies files with retry on I/O error.\n\tUsage: %s source_file dest_file", name);
    exit(-1);
}

int main(int argc, char *argv[]) {
    if(argc < 3) {
        usage();
    }
    
    const char *src = argv[1];
    const char *dst = argv[2];
    
    // 4MB
    static const size_t copyBufferLen = 4194304;
    void *copyBuffer = malloc(copyBufferLen);
    
    HANDLE hSrc = CreateFile(
        src, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if(!hSrc) {
        fprintf(stderr, "Opening source file failed with error: %i\n", GetLastError());
        return 5;
    }
    
    HANDLE hDst = CreateFile(
        dst, GENERIC_WRITE, /*No sharing*/0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
    if(!hDst) {
        fprintf(stderr, "Opening destination file failed with error: %i\n", GetLastError());
        return 7;
    }
    
    BOOL success = FALSE;
    DWORD bytesRead = 0;
    DWORD bytesWritten = 0;
    printf("Copy started\n");
    do {
        success = FALSE;
        bytesRead = 0;
        bytesWritten = 0;
        success = ReadFile(hSrc, copyBuffer, copyBufferLen, &bytesRead, NULL);
        if(!success) {
            CloseHandle(hDst);
            CloseHandle(hSrc);
            fprintf(stderr, "Reading source file failed with error: %i\n", GetLastError());
            return 15;
        }
        success = WriteFile(hDst, copyBuffer, bytesRead, &bytesWritten, NULL);
        if(!success) {
            CloseHandle(hDst);
            CloseHandle(hSrc);
            fprintf(stderr, "Reading source file failed with error: %i\n", GetLastError());
            return 17;
        }
    } while(bytesRead);
    printf("Copy complete\n");
    
    return 0;
}