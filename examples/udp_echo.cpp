#include <o.h>

using udp = o::io::net::udp_port<std::string, o::ccy::safe>;

class udp_echo : public o::io::signal_listener_app<o::ccy::safe>, public udp {
  public:
    explicit udp_echo()
        : o::io::net::udp_port<std::string, o::ccy::safe>(this->context()) {}

    void on_app_started() override {
        if (!is_running_.test_and_set()) {
            this->app_prepare();
            this->udp_bind(6000);
        }
    }

    int on_app_exit(int reason) override {
        this->udp_close();
        return 0;
    }

    void on_data_received(std::string&& data) override {
        std::this_thread::sleep_for(std::chrono::milliseconds(3));
        this->answer(std::forward<std::string>(data));
    }

    void on_data_sent() override { this->do_read(); }

    void on_udp_error(udp::error_case eca,
                      boost::system::error_code ec) override {
        std::cout << "error: " << ec.message() << std::endl;
        shutdown();
    }

    void shutdown() {
        this->sig_close();
        this->udp_close();
        this->app_allow_exit(0);
    }

    void start() { this->create_threads(4); }
    void stop() {
        this->do_run();
        this->await_threads_end();
    }

    std::atomic_flag is_running_;
};

int main() {
    udp_echo echo;
    echo.start();
    echo.stop();
}
