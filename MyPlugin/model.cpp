#include "model.h"

// GBK 转 UTF-8
string Model::GBKToUTF8(const string& gbk) {
    int wlen = MultiByteToWideChar(CP_ACP, 0, gbk.c_str(), -1, NULL, 0);
    if (wlen <= 0) return "";
    vector<wchar_t> wbuffer(wlen);
    MultiByteToWideChar(CP_ACP, 0, gbk.c_str(), -1, wbuffer.data(), wlen);
    int u8len = WideCharToMultiByte(CP_UTF8, 0, wbuffer.data(), -1, NULL, 0, NULL, NULL);
    if (u8len <= 0) return "";
    vector<char> u8buffer(u8len);
    WideCharToMultiByte(CP_UTF8, 0, wbuffer.data(), -1, u8buffer.data(), u8len, NULL, NULL);
    return string(u8buffer.data());
}

// UTF-8 转 GBK
string Model::UTF8ToGBK(const string& utf8) {
    int wlen = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, NULL, 0);
    if (wlen <= 0) return "";
    vector<wchar_t> wbuffer(wlen);
    MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, wbuffer.data(), wlen);
    int gbklen = WideCharToMultiByte(CP_ACP, 0, wbuffer.data(), -1, NULL, 0, NULL, NULL);
    if (gbklen <= 0) return "";
    vector<char> gbkbuffer(gbklen);
    WideCharToMultiByte(CP_ACP, 0, wbuffer.data(), -1, gbkbuffer.data(), gbklen, NULL, NULL);
    return string(gbkbuffer.data());
}

// 记录日志
void Model::log_message(const string& role, const string& content_utf8, ofstream& logFile) {
    if (logFile.is_open()) {
        logFile << "\n" << role << ": " << content_utf8<<"\nassistant:";
        logFile.flush();
    }
}

// **流式回调函数**（实时写入 chat_log.txt）
size_t Model::StreamCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t totalSize = size * nmemb;
    std::string chunk((char*)contents, totalSize);

    LogStreams* logs = static_cast<LogStreams*>(userp);

    try {
        json jsonObj = json::parse(chunk);
        if (jsonObj.contains("message") && jsonObj["message"].contains("content")) {
            std::string content_utf8 = jsonObj["message"]["content"].get<std::string>();

            std::cout << UTF8ToGBK(content_utf8);  // 转换后实时输出到控制台
            std::cout.flush();

            // 写入 chat_log.txt（仅记录模型回复）
            if (logs->chatLog && logs->chatLog->is_open()) {
                *(logs->chatLog) << content_utf8;
                logs->chatLog->flush();
            }
            // 同时写入 log.txt（记录用户和模型回复）
            if (logs->fullLog && logs->fullLog->is_open()) {
                *(logs->fullLog) << content_utf8;
                logs->fullLog->flush();
            }
        }
    } catch (...) {
        // 可根据需要处理异常
    }

    return totalSize;
}


// **发送请求**
void Model::send_request(const std::string& user_input_utf8, LogStreams& logs) {
    messages.push_back({{"role", "user"}, {"content", user_input_utf8}});
    json requestData = {
        {"model", model_name},
        {"messages", messages},
        {"stream", true}
    };
    std::string requestDataStr = requestData.dump();

    CURL* curl = curl_easy_init();
    if (curl) {
        struct curl_slist* headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");

        curl_easy_setopt(curl, CURLOPT_URL, api_url.c_str());
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, requestDataStr.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, requestDataStr.length());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, StreamCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &logs);

        std::cout << "助手: ";
        CURLcode res = curl_easy_perform(curl);
        std::cout << std::endl;

        if (res != CURLE_OK) {
            std::cerr << "Curl 请求失败: " << curl_easy_strerror(res) << std::endl;
        }

        if (logs.chatLog && logs.chatLog->is_open()) {
            *(logs.chatLog) << "\n";
            logs.chatLog->close();
        }
        if (logs.fullLog && logs.fullLog->is_open()) {
            *(logs.fullLog) << "\n";
            logs.fullLog->close();
        }

        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    } else {
        std::cerr << "初始化 curl 失败！" << std::endl;
    }
}

// **多轮对话**
void Model::chat() {
    SetConsoleOutputCP(936);
    cout << "多轮对话（输入 exit 退出）" << endl;

    while (true) {
        cout << "你: ";
        string user_input_gbk;
        getline(cin, user_input_gbk);
        if (user_input_gbk == "exit") break;

        string user_input_utf8 = GBKToUTF8(user_input_gbk);

        // 以截断模式打开 chat_log.txt，仅记录模型回复
        ofstream chatLog("chat_log.txt", std::ios::trunc);
        // 以追加模式打开 log.txt，记录完整对话（用户 + 模型回复）
        ofstream fullLog("log.txt", std::ios::app);

        // 将用户输入记录到完整日志中
        log_message("user", user_input_utf8, fullLog);

        // 构造 LogStreams 对象，将两个文件流指针赋值进去
        LogStreams logStreams;
        logStreams.chatLog = &chatLog;
        logStreams.fullLog = &fullLog;

        // 调用 send_request，传入 LogStreams 对象
        send_request(user_input_utf8, logStreams);
    }
}


void Model::single_chat() {
    std::string user_input_utf8;
    std::string line;
    // 读取 chat_input.txt 文件内容
    std::ifstream cursorFile("C:\\CodeBlocks_file\\chat_input.txt");
    if (!cursorFile.is_open()) {
        std::cerr << "无法打开 chat_input.txt 文件！" << std::endl;
        return;
    }
    while (getline(cursorFile, line)) {
        user_input_utf8 += line + "\n";
    }
    cursorFile.close();

    // 打开 chat_log.txt（仅记录模型回复），使用截断模式
    std::ofstream chatLog("C:\\CodeBlocks_file\\chat_log.txt", std::ios::trunc);
    // 打开 log.txt（记录完整对话），使用追加模式
    std::ofstream fullLog("C:\\CodeBlocks_file\\log.txt", std::ios::app);

    // 将用户输入记录到 log.txt 中
    log_message("user", user_input_utf8, fullLog);

    // 构造 LogStreams 结构体，将两个日志文件流传入
    LogStreams logStreams;
    logStreams.chatLog = &chatLog;
    logStreams.fullLog = &fullLog;

    // 发送请求，模型回复将同时写入 chat_log.txt 和 log.txt
    send_request(user_input_utf8, logStreams);
}

