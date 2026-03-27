#include "App.hpp"

#include "Core/Context.hpp"

int main(int, char**) {
    auto context = Core::Context::GetInstance();
    App app;

    while (!context->GetExit()) {
        switch (app.GetCurrentState()) {
            case App::State::START:
                app.Start();
                break;

            case App::State::MAIN_MENU:
                app.MainMenu();
                break;

            case App::State::GAME_INIT:
                app.GameInit();
                break;
            case App::State::UPDATE:
                app.Update();
                break;
            case App::State::END:
                app.End();
                context->SetExit(true);
                break;
        }
        context->Update();
    }
    return 0;
}
