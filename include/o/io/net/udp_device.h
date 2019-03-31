//
// This File is part of the liboh project.
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

#include "../../types.h"
#include <boost/asio.hpp>
#include <boost/optional.hpp>
#include <functional>

namespace o::io::net {

    /**
     * An UDP receiver.
     *
     * @author  Jonas Ohland
     * @date    16.03.2019
     *
     * @tparam  ThreadOption    Type of the thread option.
     */
    template <typename MessageContainer, typename ConcurrencyOption>
    class udp_device {

      public:
        struct use_v6 {};

        enum class error_case { connect, bind, read };

        udp_device() = delete;

        /**
         * Constructor
         *
         * @author  Jonas Ohland
         * @date    16.03.2019
         *
         * @param [in,out]  ctx The context.
         */
        udp_device(boost::asio::io_context& ctx) : sock_(ctx) {}

        /**
         * Handles data received signals
         *
         * @author  Jonas Ohland
         * @date    16.03.2019
         *
         * @param   parameter1  The first parameter.
         */
        virtual void on_data_received(MessageContainer&&) = 0;

        /**
         * Called when a send operation completes.
         *
         * @author  Jonas Ohland
         * @date    31.03.2019
         */
        virtual void on_data_sent() = 0;

        /**
         * Executes the UDP error action
         *
         * @author  Jonas Ohland
         * @date    16.03.2019
         *
         * @param   eca The eca.
         * @param   eco The eco.
         */
        virtual void on_udp_error(error_case eca,
                                  boost::system::error_code eco) {}

        /**
         * Bind the socket to an endpoint.
         *
         * @author  Jonas Ohland
         * @date    16.03.2019
         *
         * @param   remote_endp The remote endp.
         */
        void udp_bind(boost::asio::ip::udp::endpoint local_endp) {

            local_endp_ = local_endp;

            boost::system::error_code ec;

            if (!sock_.is_open()) sock_.open(local_endp_.protocol());

            sock_.bind(local_endp_, ec);

            if (ec) return on_udp_error(error_case::bind, ec);

            impl_udp_do_receive();
        }

        /**
         * bind the socket to the default ipv4 endpoint
         * with the specified port;
         *
         * @author  Jonas Ohland
         * @date    31.03.2019
         *
         * @param   port    The port.
         */
        void udp_bind(short port) {
            udp_bind(boost::asio::ip::udp::endpoint(
                boost::asio::ip::udp::v4(), port));
        }

        /**
         * bind the socket to the default ipv6 endpoint
         * with the specified port;
         *
         * @author  Jonas Ohland
         * @date    31.03.2019
         *
         * @param   port    The port.
         * @param   v6hint  The 6hint.
         */
        void udp_bind(short port, use_v6 v6hint) {
            udp_bind(boost::asio::ip::udp::endpoint(
                boost::asio::ip::udp::v6(), port));
        }

        /**
         * close the socket
         *
         * @author  Jonas Ohland
         * @date    16.03.2019
         */
        void udp_close() {
            if (sock_.is_open()) sock_.close();
        }

        /**
         * send a message to the specified endpoint
         *
         * @author  Jonas Ohland
         * @date    31.03.2019
         *
         * @param           endpoint        The endpoint.
         * @param [in,out]  thing_to_send   The thing to send.
         */
        void send(boost::asio::ip::udp::endpoint endpoint,
                  MessageContainer&& message) {

            MessageContainer* output_data =
                new MessageContainer(std::forward<MessageContainer>(message));

            sock_.async_send_to(
                boost::asio::buffer(output_data->data(), output_data->size()),
                endpoint,
                std::bind(&udp_device::impl_udp_on_send_done, this,
                          std::placeholders::_1, output_data));
        }

        void answer(MessageContainer&& message) {

            MessageContainer* output_data =
                new MessageContainer(std::forward<MessageContainer>(message));

            sock_.async_send_to(
                boost::asio::buffer(output_data->data(), output_data->size()),
                boost::asio::ip::udp::endpoint(last_remote()),
                std::bind(&udp_device::impl_udp_on_send_done, this,
                          std::placeholders::_1, output_data));
        }

        void do_read() { impl_udp_do_receive(); }

        boost::asio::ip::udp::endpoint& last_remote() {
            return last_remote_endp_;
        }

        boost::asio::basic_datagram_socket<boost::asio::ip::udp>& udp_sock() {
            return sock_;
        }

      private:
        // Data was received. Call the user-handler and do another receive.
        void udp_on_data_received(boost::system::error_code ec,
                                  size_t bytes_s) {

            if (ec) return on_udp_error(error_case::read, ec);

            buf_.commit(bytes_s);

            std::string out_bytes;

            on_data_received(boost::beast::buffers_to_string(buf_.data()));

            buf_.consume(bytes_s);
        }

        void impl_udp_do_receive() {
            sock_.async_receive_from(
                buf_.prepare(1024), last_remote_endp_,
                std::bind(&udp_device::udp_on_data_received, this,
                          std::placeholders::_1, std::placeholders::_2));
        }

        void impl_udp_on_send_done(boost::system::error_code ec,
                                   std::string* out_str) {

            if (ec) return on_udp_error(error_case::connect, ec);

            delete out_str;

            on_data_sent();
        }

        boost::asio::ip::udp::endpoint last_remote_endp_;
        boost::asio::ip::udp::endpoint local_endp_;

        boost::asio::streambuf buf_;
        boost::asio::ip::udp::socket sock_;
    };

    template <typename ThreadOption>
    struct udp_port_mtx_base {};

    template <>
    struct udp_port_mtx_base<o::ccy::safe> {
        std::mutex udp_port_handler_mutex_;
    };

    template <>
    struct udp_port_mtx_base<o::ccy::unsafe> {};

    template <>
    struct udp_port_mtx_base<o::ccy::none> {};

    template <typename MessageContainer, typename ConcurrencyOption>
    class udp_port : public udp_device<MessageContainer, ConcurrencyOption>,
                     public udp_port_mtx_base<ConcurrencyOption> {

        using data_handler_type = std::function<void(std::string&&)>;
        using error_handler_type =
            std::function<void(boost::system::error_code)>;

      public:
        explicit udp_port(boost::asio::io_context& ctx)
            : udp_device<MessageContainer, ConcurrencyOption>(ctx) {}

        void on_data_received(MessageContainer&& data) override {

            if (data_handler_) {

                opt_do_lock();

                data_handler_.get()(std::forward<MessageContainer>(data));

                opt_do_unlock();
            }
        }

        void on_udp_error(
            typename udp_device<MessageContainer, ConcurrencyOption>::error_case eca,
            boost::system::error_code ec) override {

            if (error_handler_) error_handler_.get()(ec);
        }

        // TODO replace with std::optional and deprecate Xcode 9 (alias buy new
        // macbook)
        boost::optional<std::function<void(MessageContainer&&)>> data_handler_;
        boost::optional<std::function<void(boost::system::error_code)>>
            error_handler_;

        inline void set_data_handler(data_handler_type&& handler) {

            opt_do_lock();

            data_handler_ =
                boost::make_optional(std::forward<data_handler_type>(handler));

            opt_do_unlock();
        }

        inline void set_error_handler(error_handler_type&& handler) {

            opt_do_lock();

            error_handler_ =
                boost::make_optional(std::forward<error_handler_type>(handler));

            opt_do_unlock();
        }

      private:
        inline void opt_do_lock() {
            if constexpr (o::ccy::is_safe<ConcurrencyOption>::value)
                this->udp_port_handler_mutex_.lock();
        }

        inline void opt_do_unlock() {
            if constexpr (o::ccy::is_safe<ConcurrencyOption>::value)
                this->udp_port_handler_mutex_.unlock();
        }
    };

} // namespace o::io::net
