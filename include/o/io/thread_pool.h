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

#pragma once

#include <boost/asio/io_context.hpp>
#include <functional>
#include <future>
#include <iostream>
#include <thread>
#include <vector>

class thread_pool {

  public:
    thread_pool(size_t threads) : thread_count_(threads) {}

    boost::asio::io_context& ioc() { return executor_; }

    void start() {
        executor_.get_executor().on_work_started();
        for (auto i = 0; i < thread_count_; ++i)
            threads_.emplace_back([this]() { this->executor_.run(); });
    }

    void stop() {
        executor_.get_executor().on_work_finished();
        for (auto& thread : threads_)
            if (thread.joinable()) thread.join();
    }

    std::vector<std::thread>& threads() { return threads_; }

    size_t thread_count_;
    std::vector<std::thread> threads_;
    boost::asio::io_context executor_;
};
