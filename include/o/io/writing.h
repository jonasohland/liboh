#pragma once

#include <queue>

namespace o::io {

    template <bool HasMtx, typename MutexType>
    class writer_mutex_base {};

    template <typename MutexType>
    class writer_mutex_base<true, MutexType> {
      protected:
        MutexType& mtx() { return mtx_; }

        MutexType mtx_;
    };

    template <typename MutexType>
    class writer_mutex_base<false, MutexType> {};

    template <typename MessageType, typename ConcurrencyOptions, typename Queue,
              typename MutexType>
    class writer
        : writer_mutex_base<ConcurrencyOptions::internal_api_safe, MutexType> {

        using ccy_options = ConcurrencyOptions;
        using queue_type = Queue;

        template <typename CCY = ConcurrencyOptions>
        std::enable_if_t<CCY::internal_api_safe> write(MessageType* msg) {}

        template <typename CCY = ConcurrencyOptions>
        std::enable_if_t<!CCY::internal_api_safe> write(MessageType* msg) {}

      private:
        Queue queue;
    };
} // namespace o::io
