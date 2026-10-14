// Pho of Fort Wayne — ordering, in four ways:
//   ./pho                 numbered menus in the terminal
//   ./pho --agent         chat with the ordering assistant (needs ANTHROPIC_API_KEY)
//   ./pho --web [port]    the web app on http://localhost:8080
//   ./pho --report        the owner's menu report (add --insights for AI advice)

#include "pho.hpp"

#include <curl/curl.h>

#include <cstdlib>
#include <iostream>
#include <string>

int main(int argc, char** argv)
{
    const std::string mode = argc > 1 ? argv[1] : "";
    curl_global_init(CURL_GLOBAL_DEFAULT);

    Session session{Cart(), Wallet(Wallet::defaultPath()), OrderLog(OrderLog::defaultPath()),
                    OrderLog::defaultCustomer()};
    int status;
    if (mode.empty()) {
        status = runClassic(session);
    } else if (mode == "--agent") {
        status = runConsoleAgent(session);
    } else if (mode == "--web") {
        const int port = argc > 2 ? std::atoi(argv[2]) : 8080;
        status = runWeb(session, port > 0 ? port : 8080);
    } else if (mode == "--report") {
        status = runReport(session.log, argc > 2 && std::string(argv[2]) == "--insights");
    } else {
        std::cerr << "Usage: pho [--agent | --web [port] | --report [--insights]]\n";
        status = 2;
    }
    curl_global_cleanup();
    return status;
}
