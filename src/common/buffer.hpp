#pragma once

#include <vector>

namespace battlespheres {

    class buffer
    {
        using size_type = std::vector<uint8_t>::size_type;

    public:
        explicit buffer(std::size_t initialsize = 4096) : _wpos(0), _rpos(0), _storage()
        {
            _storage.resize(initialsize);
        }

        buffer(const buffer &right) : _wpos(right._wpos), _rpos(right._rpos), _storage(right._storage)
        {
        }

        buffer(buffer &&right) : _wpos(right._wpos), _rpos(right._rpos), _storage(right.move())
        {
        }

        buffer &operator=(buffer const &right)
        {
            if (this != &right)
            {
                _wpos = right._wpos;
                _rpos = right._rpos;
                _storage = right._storage;
            }
            return *this;
        }

        buffer &operator=(buffer &&right)
        {
            if (this != &right)
            {
                _wpos = right._wpos;
                _rpos = right._rpos;
                _storage = right.move();
            }
            return *this;
        }

        std::vector<uint8_t> &&move()
        {
            _wpos = 0;
            _rpos = 0;
            return std::move(_storage);
        }

        void reset()
        {
            _wpos = 0;
            _rpos = 0;
        }

        void resize(size_type bytes)
        {
            _storage.resize(bytes);
        }

        uint8_t *baseptr() { return _storage.data(); }

        uint8_t *readptr() { return baseptr() + _rpos; }

        uint8_t *writeptr() { return baseptr() + _wpos; }

        void readcomplete(size_type bytes) { _rpos += bytes; }

        void writecomplete(size_type bytes) { _wpos += bytes; }

        size_type activesize() const { return _wpos - _rpos; }

        size_type remainingspace() const { return _storage.size() - _wpos; }

        size_type buffersize() const { return _storage.size(); }

        void normalize()
        {
            if (_rpos)
            {
                if (_rpos != _wpos)
                    memmove(baseptr(), readptr(), activesize());
                _wpos -= _rpos;
                _rpos = 0;
            }
        }

        void ensurefreespace()
        {
            if (remainingspace() == 0)
                _storage.resize(_storage.size() * 3 / 2);
        }

        void write(const void *data, size_t size)
        {
            if (size)
            {
                memcpy(writeptr(), data, size);
                writecomplete(size);
            }
        }

    private:
        size_type _wpos;
        size_type _rpos;
        std::vector<uint8_t> _storage;
    };
}
