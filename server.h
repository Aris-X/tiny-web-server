#pragma once
int serverStart(const char* addr, short port, const char* dir = "", const char* file = "", bool safe = true);
int serverStop();
bool serverRunning();