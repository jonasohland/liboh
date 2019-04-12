//
// This file is part of the liboh project.
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

#pragma once

#include "../ccy.h"
#include "../sequence.h"
#include "../types.h"
#include <boost/asio.hpp>

namespace o::io {

    namespace detail {

        template <bool AppManagesThreads>
        class thread_base {};

        /** base for classes that want to handle thread-creation and stuff on
         * their own */
        template <>
        class thread_base<false> {

          public:
            thread_base() = default;
            virtual ~thread_base() = default;
            thread_base(const thread_base<false>& other) = delete;
            thread_base(const thread_base<false>&& other) = delete;

            thread_base<false>*
            operator=(const thread_base<false>& other) = delete;
            thread_base<false>*
            operator=(const thread_base<false>&& other) = delete;

          protected:
            virtual void do_run() = 0;
        };

        /**
         * Base class that manages the execution of a function on one Thread.
         *
         * @author  Jonas Ohland
         */
        template <>
        class thread_base<true> {

          public:
            thread_base(const thread_base<true>& other) = delete;
            thread_base(const thread_base<true>&& other) = delete;

            thread_base<true>*
            operator=(const thread_base<true>& other) = delete;
            thread_base<true>*
            operator=(const thread_base<true>&& other) = delete;

            thread_base() = default;
            virtual ~thread_base() = default;

            /**
             * Determine on how many threads this app is running
             *
             * @returns The number of threads, this application is currently
             * running on.
             */
            size_t thread_count() const { return worker_threads_.size(); }

          protected:
            /** Wait for all threads to exit. */
            void await_threads_end() {
                std::for_each(std::begin(worker_threads_),
                              std::end(worker_threads_),
                              o::ccy::thread_join<std::thread>());
            }

            /**
             * create x new threads and begin performing work on them
             *
             * @param   num_threads (Optional) Number of threads.
             */
            void create_threads(const int num_threads = 1) {
                o::emplace_n(worker_threads_, num_threads,
                             std::bind(&thread_base::do_run, this));
            }

            /** implemented by the application to perform actual work */
            virtual void do_run() = 0;

            /**
             * Access the mutex that will be used to synchronize  actions
             * on the thread management level.
             *
             * @returns A reference to a std::mutex.
             */
            std::mutex& base_mtx() { return thread_base_mutex_; }

          private:
            std::mutex thread_base_mutex_;
            std::vector<std::thread> worker_threads_;
        };
    } // namespace detail

    /** base class for applications that want to perform io
     \snippet ioapp.cpp ioapp_base_example
     */
    template <typename ConcurrencyOptions>
    class io_app_base
        : public detail::thread_base<
              o::ccy::opt_app_manages_threads<ConcurrencyOptions>::value> {

      public:
        io_app_base() : ctx_(ConcurrencyOptions::asio_io_ctx_ccy_hint) {}
        io_app_base(int ccy_hint) : ctx_(ccy_hint) {}

        virtual ~io_app_base() = default;

        /** applications thread option (either o::threads::single or
         * o::threads::safe) */
        using ccy_options = ConcurrencyOptions;

        /** underlying io context type */
        using context_type = boost::asio::io_context;

        /** underlying executor type */
        using executor_type = boost::asio::io_context::executor_type;

        /** Run the application the callers this thread. This call will return
         * as soon as the application runs out of work. */
        template <typename Opt = ConcurrencyOptions>
        typename std::enable_if<!ccy::opt_app_manages_threads<Opt>::value,
                                int>::type
        run() {
            app_prepare();
            do_run();
            return return_code_;
        }

        template <typename Opt = ConcurrencyOptions>
        typename std::enable_if<ccy::opt_app_manages_threads<Opt>::value,
                                int>::type
        return_code() const {
            return return_code_;
        }

        /** Determine if the caller is running in one of the applications
         * threads. */
        bool call_is_in_app() {
            return context().get_executor().running_in_this_thread();
        }

        /**
         * Determine if the caller is running in the applications thread,
         * and
         * the call to the apps resources would be safe. \snippet ioapp.cpp
         * ioapp_call_is_safe_example
         *
         * @snippet ioapp.cpp ioapp_call_is_safe_example
         *
         * Will output: @code
         * call from startup method is not safe
         * call from other thread is not safe
         * call from executor is safe
         * call from timer callback is safe
         * call from exit request handler is safe
         * call from app stopped handler is not safe
         * @endcode
         *
         * @tparam  Opt Type of the option.
         *
         * @returns True if it succeeds, false if it fails.
         */
        template <typename Opt = ConcurrencyOptions>
        constexpr bool call_is_safe() {
            return o::ccy::opt_external_call_safe<Opt>::value &&
                   call_is_in_app();
        }

        /**
         * Determine if the app is running. Effectively calls thread_cout()
         * > 0
         *
         * @tparam  Opt Type of the option.
         *
         * @returns True if it the app is running, false if not.
         */
        template <typename Opt = ConcurrencyOptions>
        typename std::enable_if<o::ccy::opt_app_manages_threads<Opt>::value,
                                bool>::type
        running() const {
            return this->thread_count() > 0;
        }

      protected:
        /** is this a multithreaded application? */
        template <typename Opt = ConcurrencyOptions>
        static constexpr const bool multithreaded =
            std::is_same<Opt, ccy::safe>::value;

        /**
         * @brief   Launch the application on a specified number of threads.
         *          This function is disabled if the o::ccy::none
         *          template argument was supplied for the class.
         *
         * @tparam  Opt multi single or no thread management
         * @param   threads (Optional) The threads. Will be ignored if
         *          the o::threads::single template argument was supplied
         *
         */
        template <typename Opt = ConcurrencyOptions>
        typename std::enable_if<
            o::ccy::opt_app_manages_threads<Opt>::value>::type
        app_launch(int threads = 1) {
            app_prepare();
            this->create_threads(threads);
        }

        /** Allow the application to exit. This will call the on_app_exit
         * handler with the supplied reason code (or 0 by default) from
         * inside
         * the application and exit after that. */
        bool app_allow_exit(int reason = 0) {

            this->dispatch(
                [this, reason]() { return_code_ = this->on_app_exit(reason); });

            if (object_work_guard_.owns_work()) {
                object_work_guard_.reset();
                return true;
            }

            return false;
        }

        /** Wait for all threads owned by the app to exit. When this
         * function
         * returns, it is safe to destroy the app. */
        void app_join() const { this->await_threads_end(); }

        /** post a handler to the apps executor*/
        template <typename... Ts>
        void post(Ts&&... args) {
            boost::asio::post(ctx_, std::forward<Ts>(args)...);
        }

        /** dispatch a handler on the apps executor */
        template <typename... Ts>
        void dispatch(Ts&&... args) {
            boost::asio::dispatch(ctx_, std::forward<Ts>(args)...);
        }

        /**
         * get the underlying io context used to perform io by this app
         *
         * @returns A reference to a context_type.
         */
        context_type& context() { return ctx_; }

        /**
         * get the underlying executor used by this app
         *
         * @author  Jonas Ohland
         * @date    16.03.2019
         *
         * @returns A reference to an executor_type.
         */
        executor_type& executor() { return ctx_.get_executor(); }

        /**
         * Will be called when the app is started. This Function will be
         * called
         * once per thread. If the o::ccy::safe template argument was
         * supplied,
         * this call will be synchronized.
         *
         * @author  Jonas Ohland
         * @date    16.03.2019
         */
        virtual void on_app_started() {}

        /**
         * will be called right after the app was allowed to exit. This
         * function
         * will be called only once from one of the Applications Threads.
         *
         * @author  Jonas Ohland
         * @date    16.03.2019
         *
         * @param   reason  The reason.
         */
        virtual int on_app_exit(int reason) { return 0; }

        /**
         * Will be called when the app is stopped. This Function will be
         * called
         * once per thread. If the o::ccy::safe template argument was
         * supplied,
         * this call will be synchronized.
         *
         * @author  Jonas Ohland
         * @date    16.03.2019
         */
        virtual void on_app_stopped() {}

        /**
         * will be called when the app is started before any thread was
         * launched
         *
         * @author  Jonas Ohland
         * @date    16.03.2019
         */
        virtual void app_prepare() {}

        /**
         * Implemented to perform the actual work of this application.
         *
         * @author  Jonas Ohland
         *
         * @date    16.03.2019
         */
        virtual void do_run() override {

            if constexpr (ccy::opt_internal_call_safe<
                              ConcurrencyOptions>::value) {
                auto startup_call_lock = std::lock_guard(this->base_mtx());
                on_app_started();
            } else
                on_app_started();

            this->context().run();

            if constexpr (ccy::opt_internal_call_safe<
                              ConcurrencyOptions>::value) {
                auto startup_call_lock = std::lock_guard(this->base_mtx());
                on_app_stopped();
            } else
                on_app_stopped();
        }

      private:
        int return_code_ = -1;

        context_type ctx_;
        boost::asio::executor_work_guard<executor_type> object_work_guard_{
            ctx_.get_executor()};
    };
} // namespace o::io
