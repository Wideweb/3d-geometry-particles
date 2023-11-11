#include "AppLayer.hpp"

class App : public Engine::Application {
public:
    App(void* appInstance)
    : Engine::Application(appInstance) {
        addLayer<AppLayer>("app");
    }

    virtual ~App() {}
};

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)

#include <windows.h>

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance, PSTR cmdLine, int showCmd) {
    auto app = new App(static_cast<void*>(hInstance));
    app->run();
    delete app;
}

#elif __APPLE__ || __linux__ || __unix__ || defined(_POSIX_VERSION)

int main(int argc, char* argv[]) {
    auto app = new App(nullptr);
    app->run();
    delete app;
}

#endif
