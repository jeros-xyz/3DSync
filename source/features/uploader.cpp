#include "uploader.h"
#include <dirent.h>
#include <string>


Uploader::Uploader(Dropbox* dropbox, std::map<std::string, std::string> values) {
    _dropbox = dropbox;
    _values = values;
}

std::vector<std::string> Uploader::recurse_dir(const std::string& basepath, const std::string& additionalpath) {
    std::vector<std::string> paths;
    std::string path = basepath + additionalpath;
    DIR *dir = opendir(path.c_str());
    if (dir) {
        struct dirent *ent;
        while ((ent = readdir(dir)) != NULL) {
            std::string name = ent->d_name;
            if (name == "." || name == "..") continue;
            std::string rel_path = additionalpath + "/" + name;
            std::string abs_path = basepath + rel_path;
            struct stat st;
            if (stat(abs_path.c_str(), &st) == 0) {
                if (S_ISDIR(st.st_mode)) {
                    std::vector<std::string> subpaths = recurse_dir(basepath, rel_path);
                    paths.insert(paths.end(), subpaths.begin(), subpaths.end());
                } else if (S_ISREG(st.st_mode)) {
                    paths.push_back(rel_path);
                }
            }
        }
        closedir(dir);
    }
    return paths;
}

void Uploader::makeUpload() {
    std::map<std::pair<std::string, std::string>, std::vector<std::string>> paths;
    for(auto value : _values){
        if(value.first.rfind("paths=", 0) == 0){
            std::pair<std::string, std::string> key = std::make_pair(value.second, value.first.substr(6));
            paths[key] = recurse_dir(value.second);
        }
    }
    if((int)paths.size() > 0) _dropbox->upload2(paths);
    printf("Finish !!\n\n");
}

