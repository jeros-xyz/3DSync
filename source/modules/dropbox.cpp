#include "dropbox.h"
#include <3ds.h>
#include "../utils/ini_writer.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <curl/curl.h>
#include <string>
#include <vector>
#include <map>
#include <fstream>

Dropbox::Dropbox(std::string authorizationCode, std::string refreshToken) {
    _authorizationCode = authorizationCode;
    _refreshToken = refreshToken;
}

void Dropbox::initDropbox() {
    if (!((_refreshToken.length() > 0) && (_refreshToken != "false"))) {
        _refreshToken = getRefreshToken();
    }

    if ((_refreshToken.length() > 0) && (_refreshToken != "false")) {
        update_ini_value("/3ds/3DSync/3DSync.ini", "Dropbox", "RefreshToken", _refreshToken);
        _bearerToken = getBearerToken();

        if (_bearerToken == "false") {
            throw "Can't load Dropbox's bearerToken\n";
        }
    } else {
        throw "Can't load Dropbox's RefreshToken\n";
    }
    return;
}

/*******************************************************************************/
/**                             Globales Functions                            **/
/*******************************************************************************/

/**
 * callback for response string from dropbox (example : session_id)
 */
size_t write_to_string(void *ptr, size_t size, size_t nmemb, void *userdata) {
    std::string *str = static_cast<std::string*>(userdata);
    str->append(static_cast<char*>(ptr), size * nmemb);
    return size * nmemb;
}


std::string extract_information(const std::string& json, const std::string& field) {
    const std::string key = "\"" + field + "\":";
    size_t pos = json.find(key);
    if (pos == std::string::npos) return "false";
    pos += key.length();
    while (pos < json.size() && (json[pos] == ' ' || json[pos] == '\"')) ++pos;
    size_t end = json.find('\"', pos);
    if (end == std::string::npos) return "false";
    return json.substr(pos, end - pos);
}

/*******************************************************************************/
/**                      Initialise Dropbox OAuth tokens                      **/
/*******************************************************************************/

std::string Dropbox::getRefreshToken() {
    CURL* curl;
    CURLcode res;
    std::string responseString;
    std::string refresh_token;

    curl = curl_easy_init();
    if (curl) {
        char* code_enc = curl_easy_escape(curl, _authorizationCode.c_str(), 0);
        char* client_id_enc = curl_easy_escape(curl, APP_PUBLIC_KEY, 0);
        char* client_secret_enc = curl_easy_escape(curl, APP_SECRET_KEY, 0);

        std::string postFields = "code=" + std::string(code_enc)
            + "&grant_type=authorization_code"
            + "&client_id=" + std::string(client_id_enc)
            + "&client_secret=" + std::string(client_secret_enc);

        curl_free(code_enc);
        curl_free(client_id_enc);
        curl_free(client_secret_enc);

        curl_easy_setopt(curl, CURLOPT_URL, "https://api.dropboxapi.com/oauth2/token");
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);

        struct curl_slist* headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postFields.c_str());

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_to_string);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseString);

        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

        res = curl_easy_perform(curl);

        if (res != CURLE_OK) {
            throw "curl_easy_perform() failed: " + std::string(curl_easy_strerror(res)) + "\n\n";
        } else {
            #ifdef DEBUG
                printf("Réponse du serveur : %s\n\n", responseString.c_str());
            #endif
        }

        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);

        refresh_token = extract_information(responseString, "refresh_token");
        #ifdef DEBUG
            printf("Refresh_token : %s\n\n", refresh_token.c_str());
        #endif
    } else {
        throw "curl_easy_init() failed \n\n";
    }
    return refresh_token;
}

std::string Dropbox::getBearerToken() {
    CURL* curl;
    CURLcode res;
    std::string responseString;
    std::string bearer_token;

    curl = curl_easy_init();
    if (curl) {
        char* refresh_token_enc = curl_easy_escape(curl, _refreshToken.c_str(), 0);
        char* client_id_enc = curl_easy_escape(curl, APP_PUBLIC_KEY, 0);
        char* client_secret_enc = curl_easy_escape(curl, APP_SECRET_KEY, 0);

        std::string postFields = "refresh_token=" + std::string(refresh_token_enc)
            + "&grant_type=refresh_token"
            + "&client_id=" + std::string(client_id_enc)
            + "&client_secret=" + std::string(client_secret_enc);

        curl_free(refresh_token_enc);
        curl_free(client_id_enc);
        curl_free(client_secret_enc);

        curl_easy_setopt(curl, CURLOPT_URL, "https://api.dropboxapi.com/oauth2/token");
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);

        struct curl_slist* headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postFields.c_str());

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_to_string);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseString);

        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

        res = curl_easy_perform(curl);

        if (res != CURLE_OK) {
            throw "curl_easy_perform() failed: " + std::string(curl_easy_strerror(res)) + "\n\n";
        } else {
            #ifdef DEBUG
                printf("Réponse du serveur : %s\n\n", responseString.c_str());
            #endif
        }

        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);

        bearer_token = extract_information(responseString, "access_token");
        #ifdef DEBUG
            printf("Bearer_token : %s\n\n", bearer_token.c_str());
        #endif
    } else {
        throw "curl_easy_init() failed\n\n";
    }
            
    return bearer_token;
}

/*******************************************************************************/
/**                         Upload Dropbox Files                              **/
/*******************************************************************************/

bool upload_file_dropbox_chunked(const std::string& local_path, const std::string& bearer_token) {
    const size_t CHUNK_SIZE = 5000;
    FILE *f = fopen(local_path.c_str(), "rb");
    if (!f) {
        printf("[ERROR] Can't open local file : %s\n", local_path.c_str());
        return false;
    }
    fseek(f, 0, SEEK_END);
    long filesize = ftell(f);
    fseek(f, 0, SEEK_SET);

    std::vector<char> buf(CHUNK_SIZE);

    // 1. START SESSION
    size_t read_bytes = fread(buf.data(), 1, CHUNK_SIZE, f);
    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, ("Authorization: Bearer " + bearer_token).c_str());
    headers = curl_slist_append(headers, "Content-Type: application/octet-stream");
    headers = curl_slist_append(headers, "Dropbox-API-Arg: {\"close\":false}");

    std::string response;
    CURL *curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_URL, "https://content.dropboxapi.com/2/files/upload_session/start");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, buf.data());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, read_bytes);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_to_string);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        printf("[ERROR] upload_session/start : %s\n", curl_easy_strerror(res));
        curl_easy_cleanup(curl); fclose(f); curl_slist_free_all(headers);
        return false;
    }
    std::string session_id = extract_information(response, "session_id");
    if (session_id.empty()) {
        printf("[ERROR] session_id extract fail ! Response : %s\n", response.c_str());
        curl_easy_cleanup(curl); fclose(f); curl_slist_free_all(headers);
        return false;
    }

    #ifdef DEBUG
        printf("[INFO] Session ID : %s\n", session_id.c_str());
    #endif

    curl_easy_cleanup(curl);
    curl_slist_free_all(headers);

    long offset = read_bytes;
    // 2. APPEND CHUNKS
    while (offset < filesize) {
        size_t to_read = std::min(CHUNK_SIZE, (size_t)(filesize - offset));
        size_t n = fread(buf.data(), 1, to_read, f);
        std::string api_arg = "{\"cursor\":{\"session_id\":\"" + session_id + "\",\"offset\":" + std::to_string(offset) + "},\"close\":false}";
        struct curl_slist *headers2 = NULL;
        headers2 = curl_slist_append(headers2, ("Authorization: Bearer " + bearer_token).c_str());
        headers2 = curl_slist_append(headers2, "Content-Type: application/octet-stream");
        headers2 = curl_slist_append(headers2, ("Dropbox-API-Arg: " + api_arg).c_str());

        curl = curl_easy_init();
        curl_easy_setopt(curl, CURLOPT_URL, "https://content.dropboxapi.com/2/files/upload_session/append_v2");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers2);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, buf.data());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, n);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_to_string);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        #ifdef DEBUG
            curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0);
            curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
        #endif
        
        CURLcode res2 = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        curl_slist_free_all(headers2);
        if (res2 != CURLE_OK) {
            printf("[ERROR] upload_session/append_v2 at offset %ld : %s\n", offset, curl_easy_strerror(res2));
            fclose(f);
            return false;
        }

        #ifdef DEBUG
            printf("[INFO] Chunk envoyé à l'offset %ld (%zu octets)\n", offset, n);
        #endif
        offset += n;
    }

    // 3. FINISH SESSION
    std::string api_arg = "{\"cursor\":{\"session_id\":\"" + session_id + "\",\"offset\":" + std::to_string(offset) + "},\"commit\":{\"path\":\"" + local_path + "\",\"mode\":\"add\",\"autorename\":true,\"mute\":false}}";
    struct curl_slist *headers3 = NULL;
    headers3 = curl_slist_append(headers3, ("Authorization: Bearer " + bearer_token).c_str());
    headers3 = curl_slist_append(headers3, "Content-Type: application/octet-stream");
    headers3 = curl_slist_append(headers3, ("Dropbox-API-Arg: " + api_arg).c_str());

    std::string finish_response;
    curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_URL, "https://content.dropboxapi.com/2/files/upload_session/finish");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers3);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, NULL);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, 0);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_to_string);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &finish_response);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);

    res = curl_easy_perform(curl);

    if (res != CURLE_OK) {
        printf("[ERROR] upload_session/finish : %s\n", curl_easy_strerror(res));
        printf("[ERROR] Dropbox response : %s\n", finish_response.c_str());
        curl_easy_cleanup(curl);
        curl_slist_free_all(headers3);
        fclose(f);
        return false;
    }
    
    #ifdef DEBUG
        printf("[SUCCESS] Upload finish ! Dropbox Response : %s\n", finish_response.c_str());
    #endif

    curl_easy_cleanup(curl);
    curl_slist_free_all(headers3);
    fclose(f);
    return true;
}

void Dropbox::upload2(std::map<std::pair<std::string, std::string>, std::vector<std::string>> paths) {
    for (auto item : paths) {
        for (auto path : item.second) {
            std::string local_path = item.first.first + path;

            if (upload_file_dropbox_chunked(local_path, _bearerToken))
                printf("Upload %s Success !\n\n", local_path.c_str());
            else
                printf("Upload Fail ! %s.\n\n", local_path.c_str());
        }
    }
}

/*******************************************************************************/
/**                         Download Dropbox Files                            **/
/*******************************************************************************/
void create_directories(const std::string& file_path) {
    size_t pos = 0;
    while ((pos = file_path.find('/', pos)) != std::string::npos) {
        std::string dir = file_path.substr(0, pos);
        if (!dir.empty()) {
            int res = mkdir(dir.c_str(), 0777);
            if (res != 0 && errno != EEXIST) {
                printf("mkdir failed for '%s': %s\n", dir.c_str(), strerror(errno));
            }
        }
        ++pos;
    }
}

void Dropbox::download(std::map<std::pair<std::string, std::string>, std::vector<std::string>> paths) {
    CURL *curl = curl_easy_init();
    for (auto item : paths) {
        for (auto path : item.second) {
            printf("Downloading %s\n\n", (item.first.first + path).c_str());

            std::string local_path = item.first.first + path;
            create_directories(local_path);

            std::string args = "Dropbox-API-Arg: {\"path\":\"/" + item.first.second + path + "\"}";
            std::string auth = "Authorization: Bearer " + _bearerToken;
            struct curl_slist *headers = NULL;
            headers = curl_slist_append(headers, auth.c_str());
            headers = curl_slist_append(headers, args.c_str());
            headers = curl_slist_append(headers, "Content-Type: ");

            FILE *file = fopen(local_path.c_str(), "wb");
            if (!file) {
                printf("Failed to open %s for writing\n\n", local_path.c_str());
                curl_slist_free_all(headers);
                continue;
            }

            curl_easy_setopt(curl, CURLOPT_URL, "https://content.dropboxapi.com/2/files/download");
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);

            CURLcode rescode = curl_easy_perform(curl);
            #ifdef DEBUG
                const char *res = curl_easy_strerror(rescode);
                printf("Curl result: %s\n", res);
            #endif

            fclose(file);
            curl_slist_free_all(headers);
        }
    }
    return;
}

/*******************************************************************************/
/**                         List All Dropbox Files                            **/
/*******************************************************************************/
// Retrieves all file paths in the Dropbox response
std::vector<std::string> extract_dropbox_files(const std::string& json) {
    std::vector<std::string> files;
    size_t p = 0;
    while (true) {
        // Find ".tag"
        size_t pos_tag = json.find("\".tag\"", p);
        if (pos_tag == std::string::npos) break;
        // Go to ':' then skip spaces
        size_t colon = json.find(':', pos_tag);
        if (colon == std::string::npos) break;
        size_t val = colon + 1;
        while (val < json.size() && std::isspace(json[val])) ++val;
        // Checks if the value is "file"
        if (json.compare(val, 6, "\"file\"") == 0) {
            // Look for "path_display":" after this tag
            size_t pos_path = json.find("\"path_display\"", pos_tag);
            if (pos_path == std::string::npos) { p = colon + 1; continue; }
            pos_path = json.find(':', pos_path);
            if (pos_path == std::string::npos) { p = colon + 1; continue; }
            size_t start = json.find('"', pos_path + 1);
            if (start == std::string::npos) { p = colon + 1; continue; }
            ++start;
            size_t end = json.find('"', start);
            if (end == std::string::npos) { p = colon + 1; continue; }
            files.push_back(json.substr(start, end - start));
            p = end;
        } else {
            p = colon + 1;
        }
    }
    return files;
}

std::vector<std::string> Dropbox::listAllFiles(const std::string& dropboxPath) {
    std::vector<std::string> files;
    std::string cursor;

    std::string response = this->postAPI("https://api.dropboxapi.com/2/files/list_folder",
                                         "{\"path\":\"" + dropboxPath + "\",\"recursive\":true}");

    std::vector<std::string> page_files = extract_dropbox_files(response);
    files.insert(files.end(), page_files.begin(), page_files.end());

    while (true) {
        const char* has_more_pos = strstr(response.c_str(), "\"has_more\":");
        if (!has_more_pos) break;
        const char* p = has_more_pos + strlen("\"has_more\":");
        while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r') ++p;
        if (strncmp(p, "true", 4) != 0) break; // Si false, on sort

        const char* cursor_pos = strstr(response.c_str(), "\"cursor\":\"");
        if (!cursor_pos) break;
        cursor_pos += strlen("\"cursor\":\"");
        const char* cursor_end = strchr(cursor_pos, '"');
        if (!cursor_end) break;
        cursor = std::string(cursor_pos, cursor_end - cursor_pos);

        response = postAPI("https://api.dropboxapi.com/2/files/list_folder/continue",
                        "{\"cursor\":\"" + cursor + "\"}");
        page_files = extract_dropbox_files(response);
        files.insert(files.end(), page_files.begin(), page_files.end());
    }
    return files;
}

std::string Dropbox::postAPI(const std::string& url, const std::string& postData, bool auth) {
    std::string response;

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    std::string authHeader;
    if (auth) {
        authHeader = "Authorization: Bearer " + _bearerToken;
        headers = curl_slist_append(headers, authHeader.c_str());
    }

    CURL* curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postData.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_to_string);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            throw "curl_easy_perform() failed: " + std::string(curl_easy_strerror(res)) + "\n\n";
        } else {
            #ifdef DEBUG
                printf("Dropbox Response : %s\n\n", response.c_str());
            #endif
        }
        curl_easy_cleanup(curl);
    }
    if (headers) curl_slist_free_all(headers);

    return response;
}