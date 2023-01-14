/*
 * Copyright (c) 2022, Daniel Tabor
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#define __RES_C__
#include"resources.h"
#include<stdlib.h>
#include<string.h>

static Sint64 res_size(SDL_RWops* ctx) {
  res_data_t* res = (res_data_t*)ctx->hidden.unknown.data1;
  return res->res->len;
}

static Sint64 res_seek(SDL_RWops* ctx, Sint64 offset, int whence) {
  res_data_t* res = (res_data_t*)ctx->hidden.unknown.data1;
  int new_offset;
  
  if( whence == RW_SEEK_SET ) {
    new_offset = offset;
  }
  else if( whence == RW_SEEK_CUR ) {
    new_offset = res->offset + offset;
  }
  else if( whence == RW_SEEK_END ) {
    new_offset = res->res->len - 1 - offset;
  }
  else {
    new_offset = -1;
  }
  if( new_offset > res->res->len || new_offset < 0 ) {
    return -1;
  }
  
  res->offset = new_offset;
  return new_offset;
}

static size_t res_read(SDL_RWops* ctx, void *ptr, size_t size, size_t num) {
  res_data_t* res = (res_data_t*)ctx->hidden.unknown.data1;
  int real_size = size*num;
  while( res->offset + real_size > res->res->len ) {
    real_size = real_size - size;
  }
  if( real_size < 0 ) { return 0; }
  if( ! ptr ) { return 0; }

  memcpy(ptr,res->res->data+res->offset,real_size);
  res->offset = res->offset + real_size;
  return real_size;
}

static size_t res_write(SDL_RWops* ctx, const void *ptr, size_t size, size_t num) {
  return SDL_SetError("Can not write in this kind of RWops");
}

static int res_close(SDL_RWops* ctx) {
  res_data_t* res = (res_data_t*)ctx->hidden.unknown.data1;
  if( ctx->type != 0xdeadbeef ) {
    return SDL_SetError("Wrong king of RWops");
  }
  free(res);
  SDL_FreeRW(ctx);
  return 0;
}

SDL_RWops* getres(int index) {
  res_data_t* res;
  SDL_RWops * ctx=SDL_AllocRW();
  if( ctx == 0 ) return 0;
  ctx->size = res_size;
  ctx->seek = res_seek;
  ctx->read = res_read;
  ctx->write = res_write;
  ctx->close = res_close;
  ctx->type = 0xdeadbeef;
  res = malloc(sizeof(res_data_t));
  if( res == 0 ) {
    SDL_FreeRW(ctx);
    return 0;
  }
  res->offset = 0;
  res->res = &resources[index];
  ctx->hidden.unknown.data1 = res;
  return ctx;
}
