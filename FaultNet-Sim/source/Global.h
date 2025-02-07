#pragma once

void interfaceMain();

extern std::mutex g_PrintMutex;

static constexpr int g_NumberOfThreads = 20;
