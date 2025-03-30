#ifndef MODEL_H
#define MODEL_H

#ifdef MODEL_EXPORTS
    #define MODEL_API __declspec(dllexport)
#else
    #define MODEL_API __declspec(dllimport)
#endif

#include <streambuf>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <windows.h>

using std::string;
using std::vector;
using std::cout;
using std::cin;
using std::cerr;
using std::endl;
using std::ofstream;
using json = nlohmann::json;

// 结构体：管理两个日志文件流
struct LogStreams {
    std::ofstream* chatLog; // 记录仅模型回复的文件流（chat_log.txt）
    std::ofstream* fullLog; // 记录完整对话（用户和模型）的文件流（log.txt）
};

class Model {
private:
    string api_url;
    string model_name;
    vector<json> messages;

public:
    Model(const string& url, const string& model) : api_url(url), model_name(model) {
        messages.push_back({
            {"role", "assistant"},
            {"content", GBKToUTF8("你是一个代码助手，只会返回完整代码，不会输出解释。")}
        });
    }

    void chat();         // 多轮对话
    void single_chat();  // 单轮对话

    // 发送请求，使用流式回调，同时写入两个日志文件
    void send_request(const string& user_input_utf8, LogStreams& logs);

    // 记录日志到指定的文件流
    void log_message(const string& role, const string& content_utf8, ofstream& logFile);

    static string GBKToUTF8(const string& gbk);
    static string UTF8ToGBK(const string& utf8);
    static size_t StreamCallback(void* contents, size_t size, size_t nmemb, void* userp);
};

#endif
