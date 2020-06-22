#include <string>
#include <cstring>

using namespace std;

class fileCouple
{
private:
  string loadFile;
  string runFile;

public:
  fileCouple() {}

  fileCouple(string loadFile) {}

  fileCouple(string loadFile, string runFile) {}

  void setLoad(string loadFile) {}

  void setRun(string runFile) {}

  string getLoad() {}

  string getRun() {}
};
