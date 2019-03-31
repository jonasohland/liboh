#pragma once

namespace o::io {
    
    template <typename EndpointType>
    EndpointType set_port(EndpointType endpoint, short port) {
        auto output = endpoint;
        output.port(port);
        return output;
    }
    
} // namespace o::io
