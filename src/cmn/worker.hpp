#pragma once

int _workerMain(int argc, const char *argv[], void (*pFunc)());

#define implWorkerMain(__func__) \
   int main(int argc, const char *argv[]) { return _workerMain(argc,argv,&__func__); }
