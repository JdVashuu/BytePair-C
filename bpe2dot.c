#include<stdio.h>

#define NOB_IMPLEMENTATION
#define NOB_STRIP_PREFIX
#include "nob.h"

#include "bpe.h"

bool load_pairs(const char *file_path, Pairs *pairs, String_Builder *sb){
    if(!read_entire_file(file_path, sb))    return false;
    if(sb->count%sizeof(*pairs->items) != 0){
        fprintf(stderr, "ERROR: size of %s (%zu) must be divisible by %zu\n", file_path, sb->count, sizeof(*pairs->items));
        return false;
    }
    Pair *items = (void*)sb->items;
    size_t item_count = sb->count/sizeof(*pairs->items);
    for(size_t i = 0; i < item_count; ++i){
        da_append(pairs, items[i]);
    }
    return true;
}

void render_dot(Pairs pairs, String_Builder *sb){
    sb_append_cstr(sb, "digraph Pair{\n");
    for(uint32_t token = 0; token < pairs.count; ++token){
        if(token != pairs.items[token].l){
            sb_append_cstr(sb, temp_sprintf(" %u -> %u\n", token, pairs.items[token].l));
            sb_append_cstr(sb, temp_sprintf(" %u -> %u\n", token, pairs.items[token].r));
        }
    }
    sb_append_cstr(sb, "}\n");
}

int main(int argc, char **argv){
    const char *program_name = shift (argv, argc);

    if(argc <= 0){
        fprintf(stderr, "Usage: %s <input.bin> <output.dot>\n", program_name);
        fprintf(stderr, "ERROR: No input file provided.\n");
        return 1;
    }
    const char *input_file_path = shift (argv, argc);

    if(argc <= 0){
        fprintf(stderr, "Usage: %s <input.bin> <output.dot>\n", program_name);
        fprintf(stderr, "ERROR: No output file provided.\n");
        return 1;
    }
    const char *output_file_path = shift (argv, argc);


    Pairs pairs = {0};
    String_Builder sb = {0};

    if(!load_pairs(input_file_path, &pairs, &sb)) return 1;
    sb.count = 0;
    render_dot(pairs, &sb);

    if(!write_entire_file(output_file_path, sb.items, sb.count))     return 1;
    printf("INFO: generated %s\n", output_file_path);

    return 0;
}
