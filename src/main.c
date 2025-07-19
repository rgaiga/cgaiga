#include "chunk.h"
// #include "common.h"
#include "debug.h"
#include "virtual_machine.h"

int main(int argc, const char *argv[]) {
    init_virtual_machine();

    Chunk chunk;
    init_chunk(&chunk);

    // 1.2 + 3.4
    write_constant(&chunk, 1.2, 10);
    write_constant(&chunk, 3.4, 11);
    write_chunk(&chunk, OP_ADD, 12);

    // / 5.6
    write_constant(&chunk, 5.6, 13);
    write_chunk(&chunk, OP_DIVIDE, 14);

    // return -result
    write_chunk(&chunk, OP_NEGATE, 15);
    write_chunk(&chunk, OP_RETURN, 16);

    disassemble_chunk(&chunk, "test chunk");
    interpret(&chunk);

    free_virtual_machine();
    free_chunk(&chunk);

    return 0;
}
