#include "syn.h"
#define SYN_RECORD_REALLOC_STRIDE 1024

void syn_record_insert(struct syn_record *record, struct syn_token token)
{
    if (record->token_count % 1024)
        record->tokens = realloc(record->tokens, 
                                 sizeof(struct syn_token)*(record->token_count*1024));
    record->tokenss[record->token_count++] = token;
}

void syn_stream_insert(struct syn_stream *stream, struct syn_token token)
{
   
}
