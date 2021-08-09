#include "tcp_receiver.hh"

// Dummy implementation of a TCP receiver

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

void TCPReceiver::segment_received(const TCPSegment &seg) {
    TCPHeader header = seg.header();
    std::string data = seg.payload().copy();

    if (!header.syn && !_isn_set)
        return;
    if (header.syn) {
        _isn_set = true;
        _isn = WrappingInt32(header.seqno.raw_value());
    }
    if (_isn_set && header.fin)
        _fin = true;

    uint64_t abs_seqno = unwrap(header.seqno, _isn, _checkpoint);
    _reassembler.push_substring(data, header.syn ? 0 : abs_seqno - 1, header.fin);
    _checkpoint = _reassembler.get_ackno();
}

optional<WrappingInt32> TCPReceiver::ackno() const {
    size_t stream_ack_num = _reassembler.get_ackno();

    if (_fin && _reassembler.unassembled_bytes() == 0)
        return wrap(stream_ack_num  + 2, _isn);
    if (_isn_set)
        return wrap(stream_ack_num  + 1, _isn);

    return {};
}

size_t TCPReceiver::window_size() const { 
    return _capacity - stream_out().buffer_size();
}
