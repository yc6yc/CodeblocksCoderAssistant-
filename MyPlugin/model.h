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

// �ṹ�壺����������־�ļ���
struct LogStreams {
    std::ofstream* chatLog; // ��¼��ģ�ͻظ����ļ�����chat_log.txt��
    std::ofstream* fullLog; // ��¼�����Ի����û���ģ�ͣ����ļ�����log.txt��
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
            {"content", GBKToUTF8("����һ���������֣�ֻ�᷵���������룬����������͡�")}
        });
    }

    void chat();         // ���ֶԻ�
    void single_chat();  // ���ֶԻ�

    // ��������ʹ����ʽ�ص���ͬʱд��������־�ļ�
    void send_request(const string& user_input_utf8, LogStreams& logs);

    // ��¼��־��ָ�����ļ���
    void log_message(const string& role, const string& content_utf8, ofstream& logFile);

    static string GBKToUTF8(const string& gbk);
    static string UTF8ToGBK(const string& utf8);
    static size_t StreamCallback(void* contents, size_t size, size_t nmemb, void* userp);
};

#endif
