# gr-compress
GNURadio blocks to provide for data compression based on the zstd compression library

Note: that is OOT set is a work in progress.

## Performance
Compression ratios vary based on the data type being compressed. Smaller data types such as BYTE compress really well because there is more repetition in the data stream.  Float complex compresses poorly due to the variations.  Although this can be overcome for transmissions over UDP/TCP sinks or saving to a file by first doing a conversion such as Complex->IChar or Complex->IShort then reversing it on the other side.

Some measured statistics based on real world signals:
Interleaved Byte (SC8 / IChar): 30-70% compression depending on the data (blocks in verbose mode will tell you how you're doing) (Streaming ATA data compressed 70%, streaming radio compressed 30%)

Interleaved Short (SC16 / IShort) such as USRP Source in Complex Int16 mode: 30% compression

Complex data: 10% compression

The compress and decompress blocks have a verbose flag that can be set to yes that will intermitently print out the compression ratio you're getting which will help determine performance on your system with your data.

## Examples

There are a number of example flowgraphs in the examples directory.



## Building

Prerequisites:  Install the zstd compression library

git clone https://github.com/facebook/zstd.git  

cd zstd/lib

For some reason, the default build does not include multithreading support in the library.  Hopefully they'll fix that, but this can be enabled by editing lib/Makefile

Find the line "lib: libzstd.a libzstd".  Add a line above it to define the multithread compilation flag so the lines look like this:

lib: CPPFLAGS += -DZSTD_MULTITHREAD

lib: libzstd.a libzstd


make

sudo make install

That will provide the zstd library.  The zstd command-line tool that comes with it also does a good job of file-based compression and you can specify multiple threads with the -T# parameter.


After that, the build is pretty standard:

mkdir build

cd build

cmake ..

make

make install

ldconfig


