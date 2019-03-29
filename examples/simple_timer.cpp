#include <o.h>

//! [simpletimer_example]
using namespace std::chrono_literals;

int main() {

    // the context we will perform io on
    boost::asio::io_context ctx;

    // wait on ctx 5 secs
    o::io::wait(ctx, 5s).then([](boost::system::error_code) {
        // then print this message
        std::cout << "Times up!" << std::endl;
    });

    // actually do something
    ctx.run();

    return 0;
}

//! [simpletimer_example]
