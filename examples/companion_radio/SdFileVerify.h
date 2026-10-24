// SPDX-License-Identifier: MIT
#pragma once
#include <stddef.h>
#include <stdint.h>
#include <string.h>
namespace disaster {
// The caller closes the writer first: verify the persisted file through a fresh read handle.
template<class FS> bool sdFileMatches(FS &fs,const char *path,const uint8_t *expected,size_t length){
 auto f=fs.open(path,FILE_READ);
 bool ok=f&&!f.isDirectory()&&f.size()==length;
 uint8_t check[32];size_t offset=0;
 while(ok&&offset<length){
  size_t count=length-offset;if(count>sizeof(check))count=sizeof(check);
  ok=f.read(check,count)==count&&!memcmp(check,expected+offset,count);
  offset+=count;
 }
 if(f)f.close();
 return ok;
}
}
