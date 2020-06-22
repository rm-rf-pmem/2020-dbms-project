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
int testNum;
const regex loadReg("(\\d+)w-rw-(\\d+)-(\\d+)-load.txt");
const regex runReg("(\\d+)w-rw-(\\d+)-(\\d+)-run.txt");

// 读取文件列表
void getFileList()
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

  for (int i = 0; i < runFileList.size(); i++)
  {
    cout << "[FIND FILE] " << loadFileList[i] << ' ' << runFileList[i] << endl;
  }

  testNum = runFileList.size();
}

void loadFile(string loadFilePath, PmEHash *db = nullptr)
{
  fstream f;
  f.open(WORKLOAD + loadFilePath, ios::in);
  cout << "start to init DB with " << loadFilePath << endl;
  cout << "---------------------------------------------------" << endl;
  string cmd = "";
  string data = "";
  while (!f.eof())
  {
    f >> cmd;
    f >> data;
    string k = data.substr(0, 8);
    int v = atoi(data.substr(8).c_str());
    kv kv_pair;
    kv_pair.key = (uint64_t)k.c_str();
    kv_pair.value = v;
    db->insert(kv_pair);
  }
  f.close();
}

int main()
{
  getFileList();
  for (int i = 0; i < testNum; i++)
  {
    // PmEHash db;
    loadFile(loadFileList[i]);
  }
}
