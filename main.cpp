#include <cstdio>
#include <Windows.h>
#include <WinIoCtl.h>
using namespace std;

class ProgressBar {
public:
    ProgressBar(int width, FILE *output = stderr) : width(width), output(output)
    {
        // HACK: normally, stderr is for the progress bar, this will interfere with stdout because
        // stdout is buffered but stderr is not.  Therefore output appears out of order when mixing files.
        // This hack simply unbuffers stdout
        setbuf(stdout, 0);
        // I'm not sure why I needed this as well.
        setbuf(stderr, 0);
    }
    void useConsole() {
        fprintf(output, "\n\r");
    }
    void drawProgess(double progress) {
        int done = int(width * progress);
        // int left = width - done;
        fprintf(output, "\r%c", opn);
        int i;
        for(i = 0; i < done; i++) {
            putc(fin, output);
        }
        for(; i < width; i++) {
            putc(rem, output);
        }
        fprintf(output, "%c %03.1f%%", cls, progress*100.0);
    }
    void dropConsole() {
        putc('\n', output);
    }

private:
    int width;
    FILE *output;
    static const char fin = '+';
    static const char rem = '-';
    static const char opn = '[';
    static const char cls = ']';
};

bool getFileSize(HANDLE hFile, long long *out) {
    GET_LENGTH_INFORMATION lenInfo;
    lenInfo.Length.QuadPart = 0;
    DWORD bytesReturned;
    DWORD rc;

    // Try reading the size a file first
    rc = GetFileSizeEx(hFile, &lenInfo.Length);
    if(!rc) {
        // Try as a disk
        rc = DeviceIoControl(hFile, IOCTL_DISK_GET_LENGTH_INFO, NULL, 0, &lenInfo, sizeof lenInfo, &bytesReturned, NULL);
        if(!rc) {
            return false;
        }
    }

    *out = lenInfo.Length.QuadPart;
    return true;
}

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

    HANDLE hSrc = CreateFile(src, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if(!hSrc) {
        fprintf(stderr, "Opening source file failed with error: %i\n", GetLastError());
        return 5;
    }

    long long srcSize;
    bool gotSize = getFileSize(hSrc, &srcSize);
    double totalBytesToWrite = srcSize;
    if(!gotSize) {
        fprintf(stderr, "Getting source size failed with error: %i\n", GetLastError());
        return 6;
    }

    HANDLE hDst = CreateFile(dst, GENERIC_WRITE, /*No sharing*/0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
    if(!hDst) {
        fprintf(stderr, "Opening destination file failed with error: %i\n", GetLastError());
        return 7;
    }

    BOOL success = FALSE;
    DWORD bytesRead = 0;
    DWORD bytesWritten = 0;
    long long totalWritten = 0;
    ProgressBar progress(60);
    printf("Copy started\n");
    progress.useConsole();
    do {
        success = FALSE;
        bytesRead = 0;
        bytesWritten = 0;
        success = ReadFile(hSrc, copyBuffer, copyBufferLen, &bytesRead, NULL);
        if(!success) {
            progress.dropConsole();
            CloseHandle(hDst);
            CloseHandle(hSrc);
            fprintf(stderr, "Reading source file failed with error: %i\n", GetLastError());
            return 15;
        }
        success = WriteFile(hDst, copyBuffer, bytesRead, &bytesWritten, NULL);
        if(!success) {
            progress.dropConsole();
            CloseHandle(hDst);
            CloseHandle(hSrc);
            fprintf(stderr, "Reading source file failed with error: %i\n", GetLastError());
            return 17;
        }
        totalWritten += bytesWritten;
        progress.drawProgess(totalWritten / totalBytesToWrite);

    } while(bytesRead);
    progress.dropConsole();
    printf("Copy complete\n");

    return 0;
}