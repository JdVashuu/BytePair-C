#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define NOB_IMPLEMENTATION
#define NOB_STRIP_PREFIX
#include "nob.h"


#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"

#define swap(Type, x, y) \
do{\
    Type t = (x);\
    (x) = (y); \
    (y) = t; \
} while(0); \

typedef struct{
    uint32_t l, r;
}Pair;

typedef struct {
    Pair *items;
    size_t count;
    size_t capacity;
}Pairs;

typedef struct{
    Pair key;
    size_t value;
}Freq;

int compare_freqs(const void *a, const void *b){
    const Freq *fa = a;
    const Freq *fb = b;
    return (int)fb->value - (int)fa->value;
}

typedef struct{
    Freq *items;
    size_t count;
    size_t capacity;
}Freqs;

typedef struct{
    uint32_t *items;
    size_t count;
    size_t capacity;
}Tokens;

void render_tokens(Pairs pairs, Tokens tokens){
    for(size_t i = 0; i < tokens.count; ++i){
        uint32_t token = tokens.items[i];
        assert(token < pairs.count);
        if(pairs.items[token].l == token){
            printf("%c", token);
        }else{
            printf("[%u]", token);
        }
    }
    printf("\n");
}

void generate_dots(Pairs pairs){
    //this is an unnecessary function for generating graphs using graphwiz, no actual involvemnet in code
    printf("digraph Pair{\n");
    for(uint32_t token = 0; token < pairs.count; ++token){
        if(token != pairs.items[token].l){
            printf(" %u -> %u\n", token, pairs.items[token].l);
            printf(" %u -> %u\n", token, pairs.items[token].r);
        }
    }
    printf("}\n");
}


bool dump_pairs(const char *file_path, Pairs pairs){
    return nob_write_entire_file(file_path, pairs.items, pairs.count*sizeof(*pairs.items));
}

bool load_pairs(const char *file_path, Pairs pairs, String_Builder *sb){
    if(!read_entire_file(file_path, sb))    return false;
    if(sb->count%sizeof(*pairs.items) != 0){
       fprintf(stderr, "ERROR: size of ")
    }
}


int main(){

    const char* text = "The original BPE algorithm operates by iteratively replacing the most common contiguous sequences of characters in a target text with unused 'placeholder' bytes. The iteration ends when no sequences can be found, leaving the target text effectively compressed. Decompression can be performed by reversing this process, querying known placeholder terms against their corresponding denoted sequence, using a lookup table. In the original paper, this lookup table is encoded and stored alongside the compressed text.";
    int text_size = strlen(text);

    Freq *freq = NULL;
    Pairs pairs = {0};
    Tokens tokens_in = {0};
    Tokens tokens_out = {0};

    for(uint32_t i = 0; i < 256; i++){
        da_append(&pairs, ((Pair){.l = i}));
    }

    for(int i = 0; i < text_size; i++){
        da_append(&tokens_in, text[i]);
    }

    for(;;){
        //render_tokens(pairs, tokens_in);
        //printf("%zu\n", tokens_in.count);
        hmfree(freq);

        for(size_t i = 0; i < tokens_in.count - 1; i++){
            Pair pair = {
                .l = tokens_in.items[i],
                .r =  tokens_in.items[i + 1]
            };

            ptrdiff_t i = hmgeti(freq, pair);
            if( i < 0)  hmput(freq, pair, 1);
            else    freq[i].value += 1;
        }

        ptrdiff_t max_index = 0;
        for(ptrdiff_t i = 0; i < hmlen(freq); ++i){
            if(freq[i].value > freq[max_index].value){
                max_index = i;
            }
        }

        if(freq[max_index].value <= 1)    break;    //compression is done

        da_append(&pairs, freq[max_index].key);

        tokens_out.count = 0;
        for(size_t i = 0; i < tokens_in.count - 1;){
            if(i + 1 >= tokens_in.count){
                da_append(&tokens_out, tokens_in.items[i]);
                i += 1;
            }else{
                Pair pair = {.l = tokens_in.items[i], .r =  tokens_in.items[i + 1]};
                if(memcmp(&pair, &freq[max_index].key, sizeof(pair)) == 0){
                    da_append(&tokens_out, pairs.count - 1);
                    i += 2;
                }else{
                    da_append(&tokens_out, tokens_in.items[i]);
                    i += 1;
                }
            }
        }
        swap(Tokens, tokens_in, tokens_out);
    }

    generate_dots(pairs);
    if(!dump_pairs("pairs.bin", pairs)) return 1;



    // Freqs sorted_freqs = {0};
    // for(ptrdiff_t i = 0; i < hmlen(freq); ++i){
    //     da_append(&sorted_freqs, freq[i]);  //dynamic arrar
    // }

    // qsort(sorted_freqs.items, sorted_freqs.count, sizeof(*sorted_freqs.items), compare_freqs);

    // for(size_t i = 0; i < 10; ++i){
    //     Freq *freq = &sorted_freqs.items[i];
    //     printf("(%u, %u) => %zu\n", freq->key.l, freq->key.r, freq->value);
    // }


    return 0;
}
