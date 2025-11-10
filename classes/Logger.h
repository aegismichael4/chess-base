#pragma once

#include "../imgui/imgui.h"

#include <string>

// this class uses a singleton structure! i learned about them here:
// https://refactoring.guru/design-patterns/singleton/cpp/example

class Logger {

    private:
        Logger();
        ~Logger();

        void RenderText();
        void RenderButtons();

        void CreateAndPushEntry(ImVec4 color, std::string entryText);

    public:

        static Logger& GetInstance();

        void GameStartUp();
        void RenderGame();
        
        void LogInfo(const std::string info);
        void LogGameEvent(const std::string gameEvent);
        void LogWarning(const std::string warning);
        void LogError(const std::string error);

        void ClearLog();

};