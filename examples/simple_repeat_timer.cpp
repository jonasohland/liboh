

#include <o.h>

//! [repeattimer_example]
using namespace std::chrono_literals;

int main() {

    // the context we will perform io on
    boost::asio::io_context ctx;

    // call the repeatedly
    o::io::every(ctx, 1s).repeat(
        [cnt = 5](boost::system::error_code ec) mutable {
            // exit when counter <= 0
            bool do_exit = --cnt <= 0;

            std::cout << cnt << ((do_exit) ? " ...booom!!" : "") << std::endl;

            // or an error occurred
            return ec || do_exit;
        });

    // actually do something
    ctx.run();

    return 0;
}

//! [repeattimer_example]
