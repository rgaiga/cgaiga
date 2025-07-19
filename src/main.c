#include "chunk.h"
// #include "common.h"
#include "debug.h"

int main(int argc, const char *argv[]) {
    Chunk chunk;
    init_chunk(&chunk);

    write_constant(&chunk, 155.5, 122);
    write_chunk(&chunk, OP_RETURN, 123);

    disassemble_chunk(&chunk, "test chunk");

    return 0;
}
