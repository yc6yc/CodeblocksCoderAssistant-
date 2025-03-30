#ifndef CLAUD_MODEL_H
#define CLAUD_MODEL_H


#include <iostream>
#include <string>
#include <nlohmann/json.hpp>

class ClaudModel {
private:
    std::string api_url;
    std::string api_key;
    std::string model_name;

    static size_t writeCallback(void* contents, size_t size, size_t nmemb, void* userp);
    std::string sendRequest(const nlohmann::json& request_body);
    std::string readFile(const std::string& file_path);
    void writeFile(const std::string& file_path, const std::string& content);

public:
    ClaudModel(const std::string& url, const std::string& key, const std::string& model);

    void setAPIUrl(const std::string& url);
    void setAPIKey(const std::string& key);
    void setModelName(const std::string& model);

    std::string getAPIUrl() const;
    std::string getAPIKey() const;
    std::string getModelName() const;



    void single_chat();
    void chat();
};

#endif // CLAUD_MODEL_H
