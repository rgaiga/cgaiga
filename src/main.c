#include "chunk.h"
#include "common.h"
#include "debug.h"

int main(int argc, const char *argv[]) {
    Chunk chunk;
    init_chunk(&chunk);

    int constant_index = add_constant(&chunk, 40.0);
    write_chunk(&chunk, OP_CONSTANT, 123);
    write_chunk(&chunk, constant_index, 123);

    write_chunk(&chunk, OP_RETURN, 123);

    disassemble_chunk(&chunk, "test chunk");

    return 0;
}
