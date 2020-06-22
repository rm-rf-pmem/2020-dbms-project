// #include <pm_ehash.h>
#include "../include/pm_ehash.h"
#include "../include/data.h"
#include <fstream>
#include <iostream>
#include <dirent.h>
#include <regex>
#include <vector>
#include <string.h>
#include <cstdio>

using namespace std;

#define WORKLOAD "../workloads/"

vector<fileCouple> files;
vector<string> runFileList;
vector<string> loadFileList;
const regex loadReg("(\\d+)w-rw-(\\d+)-(\\d+)-load.txt");
const regex runReg("(\\d+)w-rw-(\\d+)-(\\d+)-run.txt");

// 读取文件列表
void loadFile()
{
  files.clear();
  runFileList.clear();
  loadFileList.clear();

  string dirname;
  DIR *dp;
  struct dirent *dirp;
  if ((dp = opendir(WORKLOAD)) == NULL)
  {
    cout << "Can't open " << WORKLOAD << endl;
  }
  while ((dirp = readdir(dp)) != NULL)
  {
    if (regex_match(dirp->d_name, loadReg))
      loadFileList.push_back(dirp->d_name);
    else if (regex_match(dirp->d_name, runReg))
      runFileList.push_back(dirp->d_name);
    // printf("[READ FILE] %s\n", dirp->d_name);
  }
  closedir(dp);

  for (auto run : runFileList)
    cout << "[RUN FILE] " << run << endl;
  for (auto load : loadFileList)
    cout << "[LOAD FILE] " << load << endl;
}

int main()
{
  loadFile();
}
