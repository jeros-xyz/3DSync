#ifndef FEATURES_DOWNLOADER_H
#define FEATURES_DOWNLOADER_H

#include "../modules/dropbox.h"
#include <map>
#include <string>
#include <vector>

class Downloader {
public:
    Downloader(Dropbox* dropbox);
    ~Downloader() {};
    void makeDownload(const std::string& dropboxFolder, const std::string& localFolder);
private:
    Dropbox* _dropbox;
    std::vector<std::string> recurse_dir(std::string basepath, std::string additionalpath = "");
};

#endif
