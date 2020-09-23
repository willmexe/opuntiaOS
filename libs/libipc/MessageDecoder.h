#pragma once
#include "Message.h"
#include <memory.h>

class MessageDecoder {
public:
    MessageDecoder() {}
    virtual ~MessageDecoder() {};

    virtual int magic() { return 0; }
    virtual unique_ptr<Message> decode(const char* buf, size_t size, size_t& decoded_msg_len) { return nullptr; }
    virtual unique_ptr<Message> handle(const Message&) { return nullptr; }
};