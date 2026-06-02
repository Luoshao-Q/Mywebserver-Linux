#include "core/server.h"
#include "log/log.h"

int main() {
    initLog("log/server.log");

    WebServer server(8080, "root");
    if (!server.start()) {
        logMessage("Æô¶¯·þÎñÆ÷Ê§°Ü", LOG_ERROR);
        return 1;
    }

    server.run();
    server.stop();
    closeLog();

    return 0;
}