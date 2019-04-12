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

#include <o.h>

struct application : o::io::io_app_base<o::ccy::strict> {

    o::io::writer<std::string, o::ccy::strict, std::queue<std::string*>,
                  std::mutex>
};

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
