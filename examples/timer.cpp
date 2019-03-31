//
// This file is part of the liboh project
//
// Copyright (c) 2019, Jonas Ohland
//
// MIT License
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <chrono>
#include <iostream>
#include <o.h>

using namespace std::chrono_literals;
namespace sys = boost::system;

using app_base = o::io::signal_listener_app<o::ccy::none>;

struct app : public app_base {

    // the app was started via the run() call
    virtual void on_app_started() override {

        // create a new timer that waits 5 seconds
        timer_ = o::io::wait(this->context(), 5s)
                     .then(o::io::timer_cb_bind(&app::handle_quit_cb, this));

        // bind a second callback to the same timer
        o::io::new_wait(timer_.lock())
            .then(o::io::timer_cb_bind(&app::other_op, this));

        // bind a third callback to the same timer
        o::io::timer_add_cb(timer_.lock(), example_callback());

        // bind via the regular boost async_wait
        // in this case, you must ensure that the timer will not
        // be deleted as long as the callback is waiting
        timer_.lock()->async_wait([capt_timer = timer_](sys::error_code) {
            std::cout << "And i was called last :(" << std::endl;
        });

        // Annoy the user every 500 milliseconds
        auto tm = o::io::every(this->context(), 500ms)
                      .repeat(o::io::timer_cb_bind(&app::annoy, this));
    }

    bool annoy(boost::system::error_code ec) {

        // Annoy
        std::cout << ((ec || stop) ? "Goodbye!" : "Hello!") << std::endl;

        // Return true (stop repeating) if the stop flag was set or ec
        // holds a value (an error ocurred, or the operation was
        // cancelled)
        return ec || stop;
    }

    // a useless function
    void other_op(boost::system::error_code) {
        std::cout << "I was called!" << std::endl;
    }

    // timer callbacks can also be functor-like objects
    struct example_callback : o::io::timer_callback {

        // will be called from the timer
        virtual void operator()(sys::error_code) override {
            std::cout << "I was called too!" << std::endl;
        }
    };

    void handle_quit_cb(boost::system::error_code ec) {
        // do nothing if ec holds a value (an error ocurred, or
        // the operation was cancelled)
        if (ec) return;

        std::cout << "Timer Callback" << std::endl;

        // allow the application to exit
        this->app_allow_exit();
    }

    // The app was allowed to exit either through a signal from the os or via
    // the app_allow_exit() call
    void on_app_exit(int reason) override {

        std::cout << "Exit reason: " << reason << std::endl;

        // stop waiting for signals
        this->sig_close();

        // set the stop flag
        stop = true;

        // cancel the exit timeout
        o::io::weak_timer_cancel(timer_);
    }

    bool stop = false;

    o::io::weak_steady_timer timer_;
};

int main() {

    app a;

    a.run();

    return 0;
}
