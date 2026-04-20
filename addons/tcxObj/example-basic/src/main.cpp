#include <TrussC.h>
#include "tcApp.h"

int main() {
    WindowSettings settings;
    settings.title = "tcxObj Example";
    settings.width = 800;
    settings.height = 600;
    return runApp<tcApp>(settings);
}
