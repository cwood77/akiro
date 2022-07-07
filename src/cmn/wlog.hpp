#pragma once
#include <ostream>

class workerLogBinding {
public:
   explicit workerLogBinding(std::wostream& s);
   ~workerLogBinding() { gSelf = NULL; }

   static std::wostream& getLog();

private:
   static workerLogBinding *gSelf;

   std::wostream& m_s;
};

inline std::wostream& getWorkerLog() { return workerLogBinding::getLog(); }
