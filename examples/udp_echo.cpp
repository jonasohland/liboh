#include <o.h>

using udp = o::io::net::datagram_device<boost::asio::ip::udp, std::string,
                                        o::ccy::safe>;

class udp_echo : public o::io::signal_listener_app<o::ccy::safe>, public udp {
  public:
    explicit udp_echo() : udp(this->context()) {}

    void on_app_started() override {
        if (!init_flag.test_and_set()) {
            this->app_prepare();
            this->dgram_sock_bind(6000);
        }
    }

    int on_app_exit(int reason) override {
        this->dgram_sock_close();
        return 0;
    }

    void on_dgram_received(std::string&& data) override {
        std::this_thread::sleep_for(std::chrono::milliseconds(3));
        this->dgram_reply(std::forward<std::string>(data));
    }

    void on_dgram_sent() override { this->do_read(); }

    void on_dgram_error(udp::error_case eca,
                        boost::system::error_code ec) override {

        if (ec != boost::asio::error::operation_aborted)
            std::cout << "error: " << ec.message() << std::endl;

        shutdown();
    }

    void shutdown() {
        this->sig_close();
        this->app_allow_exit(0);
    }

    void start() { this->create_threads(4); }
    void stop() {
        this->do_run();
        this->await_threads_end();
    }

    std::atomic_flag init_flag = ATOMIC_FLAG_INIT;
};

int main() {
    udp_echo echo;
    echo.start();
    echo.stop();
}
