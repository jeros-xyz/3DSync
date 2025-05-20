#ifndef FEATURES_UPLOADER_H
#define FEATURES_UPLOADER_H

#include "../modules/dropbox.h"

class Uploader{
    public:
        Uploader(Dropbox* dropbox, std::map<std::string, std::string> values);
        ~Uploader(){};
        void makeUpload();
    private:
        Dropbox* _dropbox;
        std::map<std::string, std::string> _values;
        std::vector<std::string> recurse_dir(const std::string& basepath, const std::string& additionalpath = "");
};

#endif
