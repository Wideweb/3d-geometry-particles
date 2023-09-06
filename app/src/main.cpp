#include "AppLayer.hpp"

class App : public Engine::Application {
  public:
    App() : Engine::Application() {
        addLayer<AppLayer>("app");
    }

    virtual ~App() {}
};

int main(int argc, char *argv[]) {
    auto app = new App();
    app->run();
    delete app;
}