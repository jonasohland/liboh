#include <o.h>

using udp = o::io::net::udp_port<std::string, o::ccy::none>;

class udp_echo : public o::io::signal_listener_app<o::ccy::none>,
                 public o::io::net::udp_port<std::string, o::ccy::none> {
  public:
    udp_echo()
        : o::io::net::udp_port<std::string, o::ccy::none>(this->context()) {}

    void on_app_started() override { this->udp_bind(6000); }

    void on_app_exit(int reason) override { this->udp_close(); }

    void on_data_received(std::string&& data) override {

        std::cout << "received data: " << data
                  << " from: " << this->last_remote().address().to_string()
                  << std::endl;

        this->udp_connect(this->last_remote());

        this->send(std::forward<std::string>(data));
    }
};

int main() {

    udp_echo echo;

    echo.run();

    return 0;
}
