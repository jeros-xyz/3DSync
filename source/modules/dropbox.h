#ifndef MODULES_DROPBOX_H
#define MODULES_DROPBOX_H

#include <string>
#include <cstring>
#include <vector>
#include <map>
#include <curl/curl.h>

class Dropbox{
    public:
        Dropbox(std::string authorizationCode, std::string refreshToken);
        ~Dropbox(){};
        void initDropbox();
        std::string getRefreshToken();
        std::string getBearerToken();
        void upload2(std::map<std::pair<std::string, std::string>, std::vector<std::string>> paths);
        void download(std::map<std::pair<std::string, std::string>, std::vector<std::string>> paths);
        std::vector<std::string> listAllFiles(const std::string& dropboxPath);
        std::string postAPI(const std::string& url, const std::string& postData, bool auth = true);
    private:
        std::string _authorizationCode;
        std::string _refreshToken;
        std::string _bearerToken;
};

#endif
