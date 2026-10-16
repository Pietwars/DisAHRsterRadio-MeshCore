// SPDX-License-Identifier: MIT
#pragma once
#include <stddef.h>
namespace disaster {
constexpr size_t ARCHIVE_LIMIT=1048576;
inline bool archiveRotate(size_t size,size_t record){return size%record!=0||size+record>ARCHIVE_LIMIT;}
}
