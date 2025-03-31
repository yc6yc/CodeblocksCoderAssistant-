# CodeblocksCoderAssistant-
This is the first C++ course practice project completed by four code novices with the help of AI, the project aims to make codeblocks, an IDE, can also be revitalized in the AI era, the project code is still in iteration, I hope you will support it.This plugin provides AI conversation functionality for the Code::Blocks Integrated Development Environment (IDE). Developers can interact with the AI system to quickly receive programming assistance, problem-solving, and code optimization suggestions. The plugin connects to an AI interface to provide real-time intelligent answers.


Code::Blocks AI 对话插件


概述
本插件为Code::Blocks集成开发环境（IDE）提供了AI对话功能，开发者可以通过与智能对话系统交互，快速获得编程帮助、问题解答及代码优化建议。插件通过与AI接口的连接，能够为开发者提供智能问答支持。

主要功能
AI对话： 与AI模型进行互动，提出问题并获取即时反馈。

本地与云端模型支持： 可选择使用本地AI模型或通过云端AI模型获取回答。

对话历史记录： 插件支持保存对话历史，用户可以查看与AI的历史互动。

简单的上下文管理： 确保对话的连续性，便于开发者进行多轮对话。

安装
前提条件
Code::Blocks IDE（需要启用插件支持）。

wxWidgets 3.1.3（用于构建插件的图形界面）。

MSYS2（用于下载并安装必要的库，libcurl 8.11.1和nolhman 3.11.3）。

AI接口： 可以选择使用云端AI模型（如OpenAI的GPT）或本地模型（如Ollama）。

安装步骤
下载并编译 wxWidgets 3.1.3。

安装 Code::Blocks 20.03 或更高版本。

安装 MSYS2，以便下载和管理必要的库。

克隆项目仓库：

git clone https://github.com/yc6yc/CodeblocksCoderAssistant-.git
按照仓库中的安装指南将插件安装到Code::Blocks中。

使用方法
1.启动与配置
启动Code::Blocks并在“插件管理器”中启用 AI对话插件。

启动插件后，按 Ctrl+M 打开模型选择对话框，选择本地或云端模型。

配置所选模型的API信息（根据所选模型可能需要提供API密钥等配置信息）。

2.与AI对话
启动插件后，按 Ctrl+K 查看并访问对话历史记录。

在插件窗口中输入问题或代码，AI会处理并返回回答。

用户可以多次交互，插件会记录对话内容并确保上下文的连续性。

根据配置的模型，AI可能会给出不同的回答（本地模型或云端模型）。

3.对话历史管理
每次与AI对话后，插件会将对话记录保存到一个C:\codeblocksfile\log.txt文件中。您可以随时查看与AI的交互历史。

4.错误处理
插件会自动处理网络延迟或连接失败的情况，并提供相应的提示信息。

确保网络连接正常，云端模型需要稳定的互联网连接。

5.故障排除
AI接口无法连接： 请检查模型API的配置，确保API密钥正确并且网络连接正常。

对话历史无法保存： 确保插件具有写入日志文件的权限，并检查日志文件路径配置是否正确。

AI对话未响应： 检查网络延迟或API调用错误，确保模型配置和API调用符合要求。

6.未来改进
增强对话历史的管理，支持永久保存和多设备同步。

提供更多的AI模型选择，扩展支持更多种类的本地与云端模型。

优化AI对话的上下文管理，使其支持更复杂的编程任务和问题解答。

贡献
欢迎开源社区的贡献者参与该插件的开发。请通过Fork仓库，进行功能改进并提交Pull Requests。

许可证
本项目采用MIT许可证。

联系方式
如有任何问题或反馈，欢迎提交Issue或直接联系仓库所有者。
