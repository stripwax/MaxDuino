// SdFat stub for CLI builds — wraps FILE* for compatibility with MaxDuino
#ifndef SDFAT_H_CLISTUB
#define SDFAT_H_CLISTUB

#include <cstdio>
#include <cstring>
#include <cstdint>

#define O_RDONLY 0

class SdFat {};

class SdBaseFile {
public:
    SdBaseFile() : _file(nullptr), _size(0) {}

    ~SdBaseFile() { close(); }

    bool seekSet(unsigned long pos) {
        if (!_file) return false;
        if (fseek(_file, pos, SEEK_SET) != 0) return false;
        return true;
    }

    int read(void* buf, unsigned int n) {
        if (!_file) return 0;
        return (int)fread(buf, 1, n, _file);
    }

    void close() {
        if (_file) { fclose(_file); _file = nullptr; }
    }

    bool open(const char* path, unsigned char flags) {
        const char* mode = (flags & O_RDONLY) ? "rb" : "rb";
        _file = fopen(path, mode);
        if (_file) {
            fseek(_file, 0, SEEK_END);
            _size = ftell(_file);
            fseek(_file, 0, SEEK_SET);
            return true;
        }
        return false;
    }

    unsigned long fileSize() const { return _size; }

private:
    FILE* _file;
    unsigned long _size;
};

#endif // SDFAT_H_CLISTUB
