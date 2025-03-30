#include "ClaudModel.h"
#include <iostream>
#include <fstream>
#include <curl/curl.h>
#include <thread>
#include <chrono>
#include"model.h"
using json = nlohmann::json;

const std::string INPUT_FILE = "chat_input.txt";
const std::string OUTPUT_FILE = "chat_log.txt";
const std::string SINGLE_INPUT_FILE = "C:\\CodeBlocks_file\\chat_input.txt";
const std::string SINGLE_OUTPUT_FILE = "C:\\CodeBlocks_file\\chat_log.txt";

ClaudModel::ClaudModel(const std::string& url, const std::string& key, const std::string& model)
    : api_url(url), api_key(key), model_name(model) {}

void ClaudModel::setAPIUrl(const std::string& url) { api_url = url; }
void ClaudModel::setAPIKey(const std::string& key) { api_key = key; }
void ClaudModel::setModelName(const std::string& model) { model_name = model; }

std::string ClaudModel::getAPIUrl() const { return api_url; }
std::string ClaudModel::getAPIKey() const { return api_key; }
std::string ClaudModel::getModelName() const { return model_name; }

size_t ClaudModel::writeCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

std::string ClaudModel::readFile(const std::string& file_path) {
    std::ifstream input_file(file_path);
    if (!input_file) return "";

    std::string content((std::istreambuf_iterator<char>(input_file)), std::istreambuf_iterator<char>());
    input_file.close();


    return content;
}

void ClaudModel::writeFile(const std::string& file_path, const std::string& content) {
   std::ofstream output_file(file_path, std::ios::trunc);
    if (output_file) {
        output_file << content << std::endl;
    }
}

std::string ClaudModel::sendRequest(const json& request_body) {
    std::string response_data;
    std::string json_str = request_body.dump();

    CURL* curl = curl_easy_init();
    if (curl) {
        struct curl_slist* headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        headers = curl_slist_append(headers, "Accept: application/json");
        std::string auth_header = "Authorization: Bearer " + api_key;
        headers = curl_slist_append(headers, auth_header.c_str());

        curl_easy_setopt(curl, CURLOPT_URL, api_url.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_str.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_data);

        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            std::cerr << "Request failed: " << curl_easy_strerror(res) << std::endl;
        }

        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    }
    return response_data;
}

void ClaudModel::single_chat() {
    std::string user_input = readFile(SINGLE_INPUT_FILE);
    if (user_input.empty()) return;

    json request_body = {
        {"messages", json::array({
            {{"role", "system"}, {"content", Model::GBKToUTF8("请补全以下代码:")}},
            {{"role", "user"}, {"content", user_input}}
        })},
        {"model", model_name},
        {"max_tokens", 2048},
        {"temperature", 1},
        {"top_p", 1}
    };

    std::string response_data = sendRequest(request_body);
    std::string assistant_reply = "（解析失败）";

    try {
        json response_json = json::parse(response_data);
        if (response_json.contains("choices")) {
            assistant_reply = response_json["choices"][0]["message"]["content"];
        }
    } catch (const std::exception& e) {
        std::cerr << "JSON Parse Error: " << e.what() << std::endl;
    }

    // 仅写入 assistant 回复到 single_chat_log.txt
    writeFile(SINGLE_OUTPUT_FILE, assistant_reply);

    // 追加用户输入和 AI 回复到 log.txt
    std::ofstream log_file("C:\\CodeBlocks_file\\log.txt", std::ios::app);
    if (log_file) {
        log_file << "User: " << user_input << "\n";
        log_file << "Assistant: " << assistant_reply << "\n";
        log_file << "------------------------------\n";
    }

    std::cout << "[单次对话] Assistant: " << assistant_reply << std::endl;
}

void ClaudModel::chat() {
    json conversation;
    conversation["messages"] = json::array();
    conversation["messages"].push_back({
        {"role", "system"},
{"content", Model::GBKToUTF8(R"(   你是一个代码智能助手，你的任务是采用git merge样式的格式修改/替换代码，帮助用户解决代码问题，如生成代码、写代码注释、代码改错、代码补全等，并为用户讲解代码的含义。
首先，请仔细阅读以下原始代码：
<code>
{{CODE}}
</code>
接下来，请查看以下任务描述：
<task_description>
{{TASK_DESCRIPTION}}
</task_description>
在进行代码修改时，请遵循以下要求：
1. 采用git merge样式的格式标记原始代码和修改后的代码。
2. 确保修改后的代码符合任务描述的要求。
3. 尽量保持代码的原有结构和风格。
请先在<思考>标签中详细分析如何进行代码修改，并说明修改的依据。然后在<修改后的代码>标签中给出修改后的代码，使用git merge样式的格式。最后，在<代码讲解>标签中详细讲解修改后的代码的含义。
<思考>
[在此详细说明你对代码修改的分析和依据]
</思考>
<修改后的代码>
[在此给出修改后的代码，使用git merge样式的格式]
</修改后的代码>
<代码讲解>
[在此详细讲解修改后的代码的含义]
</代码讲解>
)")}
    });

    std::string last_input = "";

    while (true) {


        std::string user_input = readFile(INPUT_FILE);
        if (user_input.empty() || user_input == last_input) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            continue;
        }
        last_input = user_input;

        conversation["messages"].push_back({{"role", "user"}, {"content", user_input}});

        json request_body = {
            {"messages", conversation["messages"]},
            {"model", model_name},
            {"max_tokens", 2048},
            {"temperature", 1},
            {"top_p", 1}
        };

        std::string response_data = sendRequest(request_body);
        std::string assistant_reply = "      ʧ ܣ ";

        try {
            json response_json = json::parse(response_data);
            if (response_json.contains("choices")) {
                assistant_reply = response_json["choices"][0]["message"]["content"];
            }
        } catch (const std::exception& e) {
            std::cerr << "JSON         : " << e.what() << std::endl;
        }

        std::cout << "[   ֶԻ ] Assistant: " << assistant_reply << std::endl;
        writeFile(OUTPUT_FILE, assistant_reply);

        conversation["messages"].push_back({{"role", "assistant"}, {"content", assistant_reply}});
    }
}
