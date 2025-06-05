#include "downloader.h"
#include <dirent.h>
#include <sys/stat.h>
#include <cstring>

Downloader::Downloader(Dropbox* dropbox) {
    _dropbox = dropbox;
}

std::vector<std::string> Downloader::recurse_dir(std::string basepath, std::string additionalpath) {
    std::vector<std::string> paths;
    DIR *dir;
    struct dirent *ent;
    std::string path = basepath + additionalpath;
    if ((dir = opendir(path.c_str())) != NULL) {
        while ((ent = readdir(dir)) != NULL) {
            std::string readpath = path + "/" + ent->d_name;
            if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0)
                continue;
            struct stat st;
            if (stat(readpath.c_str(), &st) == 0 && S_ISDIR(st.st_mode)) {
                std::vector<std::string> recurse = recurse_dir(basepath, additionalpath + "/" + ent->d_name);
                paths.insert(paths.end(), recurse.begin(), recurse.end());
            } else {
                if (additionalpath != "")
                    paths.push_back(additionalpath + "/" + ent->d_name);
                else
                    paths.push_back(ent->d_name);
            }
        }
        closedir(dir);
    } else {
        if (additionalpath != "")
            paths.push_back(additionalpath);
        else
            printf("Folder %s not found\n\n", basepath.c_str());
    }
    return paths;
}

void Downloader::makeDownload(const std::string& dropboxFolder, const std::string& localFolder) {
    std::vector<std::string> files = _dropbox->listAllFiles(dropboxFolder);

    std::map<std::pair<std::string, std::string>, std::vector<std::string>> paths;
    std::vector<std::string> relativeFiles;

    for (const auto& file : files) {
        std::string rel = file.substr(dropboxFolder.length());
        if (!rel.empty() && rel[0] == '/') rel = rel.substr(1);
        relativeFiles.push_back(rel);
    }
    paths[std::make_pair(localFolder, dropboxFolder)] = relativeFiles;
    _dropbox->download(paths);
}