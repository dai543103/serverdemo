#pragma once

#include "common/tcpchannel.h"
#include "soap.pb.h"

struct netpacket : public battlespheres::memory_block
{
    netpacket(int cmd, const google::protobuf::Message& message)
        : memory_block(4 + 4 + message.ByteSize())
    {
        assert(buf_.size() == size_t(4 + 4 + message.ByteSize()));
        uint32_t value = 4 + message.ByteSize();
        memcpy(&buf_[0], &value, 4);
        value = cmd;
        memcpy(&buf_[4], &value, 4);
        if (message.ByteSize() > 0)
            message.SerializeToArray(&buf_[8], message.ByteSize());
    }

    netpacket(int cmd, const char* data, size_t size)
        : memory_block(4 + 4 + size)
    {
        assert(buf_.size() == size_t(4 + 4 + size));
        uint32_t value = 4 + size;
        memcpy(&buf_[0], &value, 4);
        value = cmd;
        memcpy(&buf_[4], &value, 4);
        if (size > 0)
            memcpy(&buf_[8], data, size);
    }

    static std::shared_ptr<netpacket> create(soapproto::msgId msgId, const google::protobuf::Message& message)
    {
        return std::make_shared<netpacket>(msgId, message);
    }

    static std::shared_ptr<netpacket> create(soapproto::msgId msgId, const char* data = nullptr, size_t size = 0)
    {
        return std::make_shared<netpacket>(msgId, data, size);
    }
};

using netpacketptr = std::shared_ptr<netpacket>;

