#include "../pch.h"

#include <dirent.h> // linux only?
#include <vector>

#define MAX_FILE_PATH_LEN 512

//return a pointer the the first character following the last '.' in the c_string
//  if no '.' appears, will return nullptr
const char* getFileType(const char* full_path);

struct FileFilterAny {
    bool operator()(const char* c){
        return true;
    }
};

struct FileFilterExtension {
    std::string extType;
    FileFilterExtension(std::string ext) : extType(ext){};
    FileFilterExtension(const char* ext) : extType(ext){};

    bool operator()(const char* c){
        const char* c_ext = getFileType(c);
        if(c_ext == nullptr){
            return extType == "";
        }
        return (strcmp(c_ext, extType.c_str())) == 0;
    }
};

//returns the number of files resolved and updates sub_path_out where
//  with all files such that "root_path/sub_path_out[n]" is the n-th full path
//  and that the full path passes the provided comparator
//  non-recursive
template <class Comparator=FileFilterAny>
unsigned int getAllFiles(const char* root_path, std::vector<std::string>& sub_path_out, Comparator comp = Comparator()){
    char fullPath[MAX_FILE_PATH_LEN];
    unsigned int passedFiles = 0;
    size_t maxFileLen = MAX_FILE_PATH_LEN - strlen(root_path) - 2; //1 for '/', 1 for null
    // linux only?
    auto dir = opendir(root_path);
    if(dir){
        while (auto f = readdir(dir)){
            // size_t l = strlen(f->d_name);

            //build full path name
            if(strlen(f->d_name) > maxFileLen){
                //just skip??
                continue;
            }
            snprintf(fullPath, MAX_FILE_PATH_LEN, "%s/%s", root_path, f->d_name);
            if(comp(fullPath)){
                sub_path_out.emplace_back((f->d_name));
                passedFiles++;
            }
        }
    }
    //windows mode?
    //end
    return passedFiles;
}

