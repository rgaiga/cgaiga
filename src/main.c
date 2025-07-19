#include "chunk.h"
// #include "common.h"
#include "debug.h"
#include "virtual_machine.h"

int main(int argc, const char *argv[]) {
    init_virtual_machine();

    Chunk chunk;
    init_chunk(&chunk);

    write_constant(&chunk, 155.5, 122);
    write_chunk(&chunk, OP_NEGATE, 123);
    write_chunk(&chunk, OP_RETURN, 124);

    disassemble_chunk(&chunk, "test chunk");
    interpret(&chunk);

    free_virtual_machine();
    free_chunk(&chunk);

    return 0;
}
