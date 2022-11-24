#include "wrapping_integers.hh"

// Dummy implementation of a 32-bit wrapping integer

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

//! Transform an "absolute" 64-bit sequence number (zero-indexed) into a WrappingInt32
//! \param n The input absolute 64-bit sequence number
//! \param isn The initial sequence number
WrappingInt32 wrap(uint64_t n, WrappingInt32 isn) {
    uint64_t isn_64 = isn.raw_value();
    n += isn_64;
    uint32_t ret = n % (uint64_t(UINT32_MAX) + 1);

    return WrappingInt32{ret};
}

//! Transform a WrappingInt32 into an "absolute" 64-bit sequence number (zero-indexed)
//! \param n The relative sequence number
//! \param isn The initial sequence number
//! \param checkpoint A recent absolute 64-bit sequence number
//! \returns the 64-bit sequence number that wraps to `n` and is closest to `checkpoint`
//!
//! \note Each of the two streams of the TCP connection has its own ISN. One stream
//! runs from the local TCPSender to the remote TCPReceiver and has one ISN,
//! and the other stream runs from the remote TCPSender to the local TCPReceiver and
//! has a different ISN.
uint64_t unwrap(WrappingInt32 n, WrappingInt32 isn, uint64_t checkpoint) {
    uint64_t raw_seq = n.raw_value();
    if (n.raw_value() < isn.raw_value()) {
        raw_seq += UINT32_MAX - isn.raw_value() + 1;
    } else {
        raw_seq -= isn.raw_value();
    }

    if (raw_seq > checkpoint) {
        return raw_seq;
    }

    uint64_t q = (checkpoint - raw_seq)/(uint64_t(UINT32_MAX) + 1);
    if ((q * (uint64_t(UINT32_MAX)+1) + raw_seq - checkpoint) * -1 < (q+1) * (uint64_t(UINT32_MAX)+1) + raw_seq - checkpoint) {
        return q * (uint64_t(UINT32_MAX)+1) + raw_seq;
    }

    return (q+1) * (uint64_t(UINT32_MAX)+1) + raw_seq;
}
