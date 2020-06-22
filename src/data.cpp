#include "data.h"

class fileCouple
{
private:
  string loadFile;
  string runFile;

public:
  fileCouple()
  {
    this->loadFile = "";
    this->runFile = "";
  }

  fileCouple(string loadFile)
  {
    this->loadFile = loadFile;
    this->runFile = "";
  }

  fileCouple(string loadFile, string runFile)
  {
    this->loadFile = loadFile;
    this->runFile = runFile;
  }

  void setLoad(string loadFile)
  {
    this->loadFile = loadFile;
  }

  void setRun(string runFile)
  {
    this->runFile = runFile;
  }

  string getLoad()
  {
    return this->loadFile;
  }

  string getRun()
  {
    return this->runFile;
  }
};
