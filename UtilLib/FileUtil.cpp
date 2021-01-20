#include "FileUtil.h"

const char* getFileType(const char* full_path){
    const char * c = strchr(full_path, '.');
    if(c == nullptr){
        return nullptr;
    }
    const char * e = c;
    while((e = strchr(e+1, '.'))){
        c = e;
    }
    return c;
}