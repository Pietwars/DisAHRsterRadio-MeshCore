#pragma once
#include <stdint.h>
#include <stddef.h>
// Implemented by main.cpp, so the display never calls HTTP handlers or owns a radio.
bool deviceQuickTarget(unsigned index, char* id, size_t id_size, char* name, size_t name_size);
bool deviceQuickSend(const char* id, const char* text, uint32_t stamp, char* status, size_t status_size);
void deviceQuickStatus(char* status, size_t size);
