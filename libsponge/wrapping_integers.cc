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
    return isn + n;
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
    /*
        checkpoint is within +/- 2^31 of the correct abs seqno, so the distance from
        checkpoint to n creates no problem of overflowing. If wrap(checkpoint, isn)
        is large(2^32-1), and n is small(0), cp2n still produce correct distance
        because for int32_t the MSB is sign bit. But when it is converted 
        back to unsigned, (0-2^32+1) exceeds the expected answer(1) by -2^32.
    */
    int32_t cp2n = n - wrap(checkpoint, isn);
    int64_t abs_seqno = checkpoint + cp2n;
    return abs_seqno >= 0 ? abs_seqno : abs_seqno + (1UL << 32);
}
