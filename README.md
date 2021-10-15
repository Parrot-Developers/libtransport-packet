# libtransport-packet - Transport packet library

libtransport-packet is a C library to handle network transport packets.

## Dependencies

The library depends on the following Alchemy modules:

* libulog
* libfutils
* libpomp

## Building

Building is activated by enabling _libtransport-packet_ in the Alchemy build
configuration.

## Operation

### Threading model

The API functions are not thread safe and should always be called from the
same thread, or it is the caller's resposibility to synchronize calls if
multiple threads are used.
