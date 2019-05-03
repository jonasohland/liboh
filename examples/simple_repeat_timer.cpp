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
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

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
