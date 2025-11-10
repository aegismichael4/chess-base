#include "Logger.h"

#include <iostream>
#include <list>
#include <fstream>

static Logger* instance;
std::ofstream logFile;

struct Entry { int num; ImVec4 color; std::string text; };
int entryNum = 1;

std::list<Entry> fullLog;

Logger::Logger() {
    logFile.open("LoggerOutput.txt");
    logFile << "test" << std::endl;
}

Logger& Logger::GetInstance() {

    if (!instance) {
        instance = new Logger();
    }
    return *instance;
}

Logger::~Logger() {
    logFile.close();
}

void Logger::GameStartUp() {}

void Logger::RenderGame() {

    ImGui::Begin("Game Log");

    RenderButtons();
    RenderText();

    ImGui::End();
}

void Logger::RenderText() {

    ImGui::TextUnformatted("\n");

    for (Entry entry : fullLog) {
        
        ImGui::TextColored(entry.color, (std::to_string(entry.num) + " " + entry.text).c_str());
    }

    ImGui::LogFinish();

}

void Logger::RenderButtons() {

    ImGui::BeginGroup();

    if (ImGui::Button("Clear"))
        ClearLog();

    ImGui::SameLine();
    
    if (ImGui::Button("Test Info")) 
        LogInfo("test");

    ImGui::SameLine();

    if (ImGui::Button("Test Game Event")) 
        LogGameEvent("test");

    ImGui::SameLine();
    
    if (ImGui::Button("Test Warning")) 
        LogWarning("test");

    ImGui::SameLine();

    if (ImGui::Button("Test Error")) 
        LogError("test");
    

    ImGui::EndGroup();
}

void Logger::LogInfo(const std::string info) {

    CreateAndPushEntry(ImVec4(1, 1, 1, 1), "[info] " + info + '\n');
}

void Logger::LogGameEvent(const std::string gameEvent) {

    CreateAndPushEntry( ImVec4(0.5f, 1, 0.5f, 1), "[game event] " + gameEvent + '\n');
}

void Logger::LogWarning(const std::string warning) {

    CreateAndPushEntry( ImVec4(1, 1, 0.5f, 1), "[warning] " + warning + '\n');
}

void Logger::LogError(const std::string error) {
    CreateAndPushEntry(ImVec4(1, 0.2f, 0.2f, 1), "[error] " + error + '\n');
}

void Logger::CreateAndPushEntry(ImVec4 color, std::string entryText) {

    Entry entry;
    entry.num = entryNum;
    entryNum++;
    entry.color = color;
    entry.text = entryText;
    fullLog.push_front(entry);

    logFile << entry.text;
}

void Logger::ClearLog() {

    fullLog.clear();
    entryNum = 1;
}