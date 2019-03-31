#include <o.h>

using udp = o::io::net::udp_port<std::string, o::ccy::none>;

class udp_echo : public o::io::signal_listener_app<o::ccy::none>, public udp {
  public:
    explicit udp_echo()
        : o::io::net::udp_port<std::string, o::ccy::none>(this->context()) {}

    void on_app_started() override { this->udp_bind(6000); }

    int on_app_exit(int reason) override { this->udp_close(); return 0; }

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
};

int main() {

    udp_echo echo;

    return echo.run();;
}
