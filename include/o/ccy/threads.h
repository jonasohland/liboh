#pragma once

namespace o::ccy {
    
    template<typename Thread>
    struct thread_join {
        void operator()(Thread& thread) const {
            if(thread.joinable())
                thread.join();
        }
    };
    
}
