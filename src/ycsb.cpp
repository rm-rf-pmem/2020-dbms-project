// #include <pm_ehash.h>
#include "../include/pm_ehash.h"
#include <fstream>
#include <iostream>
#include <dirent.h>
#include <regex>
#include <vector>
#include <string.h>
#include <cstdio>

using namespace std;

#define WORKLOAD "../workloads/"

class fileCouple {
private:
  string loadFile;
  string runFile;
  regex loadReg;

public:
  fileCouple() {
    this->loadFile = "";
    this->runFile = "";
  }

  fileCouple(string loadFile) {
    this->loadFile = loadFile;
    this->runFile = "";
  }

  void setLoad(string loadFile) {
    this->loadFile = loadFile;
  }

  void setRun(string runFile) {
    this->runFile = runFile;
  }

  bool match(string name) {
    return regex_match(name, this->loadReg);
  }

  string getLoad() {
    return this->loadFile;
  }

  string getRun() {
    return this->runFile;
  }
};

vector<fileCouple> files;
vector<string> runFileList;
const regex loadReg("(\\d+)w-rw-(\\d+)-(\\d+)-load.txt");
const regex runReg("(\\d+)w-rw-(\\d+)-(\\d+)-run.txt");


// 读取文件列表
void loadFile() {
  string dirname;
  DIR *dp;
  struct dirent *dirp;
  if((dp = opendir(WORKLOAD)) == NULL)
  {
    cout << "Can't open " << WORKLOAD << endl;
  }
  while((dirp = readdir(dp)) != NULL)
  {
    if (regex_match(dirp->d_name, loadReg)) 
      files.push_back(* new fileCouple(dirp->d_name));
    else if (regex_match(dirp->d_name, runReg))
      runFileList.push_back(dirp->d_name);
    printf("[READ FILE] %s\n", dirp->d_name);
  }
  closedir(dp);

  for (auto run: runFileList) {
    for (auto load: files) {
      string loadfile = load.getLoad();
      if (loadfile.substr(0, loadfile.length() - 9) == run.substr(0, run.length() - 8)) {
        load.setRun(run);
      }
    }
  }

  for (auto file: files) {
    printf("[FIND FILE] %s %s\n", file.getLoad().c_str(), file.getRun().c_str());
  }
}

int main()
{
  loadFile();
}
