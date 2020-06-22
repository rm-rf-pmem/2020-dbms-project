// #include <pm_ehash.h>
#include "../include/pm_ehash.h"
#include <fstream>
#include <iostream>
#include <dirent.h>
#include <regex>
#include <vector>
#include <string.h>
#include <cstdio>
#include <time.h>

using namespace std;

#define WORKLOAD "../workloads/"

vector<string> runFileList;
vector<string> loadFileList;
int testNum;
const regex loadReg("(\\d+)w-rw-(\\d+)-(\\d+)-load.txt");
const regex runReg("(\\d+)w-rw-(\\d+)-(\\d+)-run.txt");

uint64_t stringTo64(string str)
{
  uint64_t res = 0;
  for (int i = 0; i < str.length(); i++)
  {
    uint8_t temp = str[i];
    res = res << 8;
    res += temp;
    // printf("%d\t%d\t%ld\n", str[i], temp, res);
  }

  return res;
}

// 读取文件列表
void getFileList()
{
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
  cout << "[STATUS] start to init DB with " << loadFilePath << endl;
  cout << "---------------------------------------------------" << endl;
  string cmd = "";
  string data = "";
  time_t start = clock();
  long long dataNum = 0;
  while (!f.eof())
  {
    cmd = data = "";
    f >> cmd;
    f >> data;
    string k = data.substr(0, 8);
    int v = atoi(data.substr(8).c_str());
    kv kv_pair;
    kv_pair.key = stringTo64(k);
    kv_pair.value = v;
    // 测试时注释掉
    // db->insert(kv_pair);
    dataNum++;
    if (dataNum % 10000 == 0)
      cout << "[STATUS] has load " << dataNum << " data" << endl;
  }
  time_t end = clock();
  cout << "[STATUS] Init DB finished, use time " << ((double)end - start) / CLOCKS_PER_SEC << "ms" << endl
       << endl
       << endl;
  f.close();
}

void runFile(string runFilePath, PmEHash *db = nullptr)
{
  fstream f;
  f.open(WORKLOAD + runFilePath, ios::in);
  cout << "[STATUS] start to run DB with " << runFilePath << endl;
  cout << "---------------------------------------------------" << endl;
  string cmd = "";
  string data = "";
  time_t start = clock();
  long long cmdNum = 0;

  while (!f.eof())
  {
    cmd = data = "";
    f >> cmd;
    f >> data;
    string k = data.substr(0, 8);
    int v = atoi(data.substr(8).c_str());
    kv kv_pair;
    kv_pair.key = stringTo64(k);
    kv_pair.value = v;
    uint64_t res = 0;
    if (cmd == "READ")
    {
      db->search(kv_pair.key, res);
    }
    else if (cmd == "INSERT")
    {
      db->insert(kv_pair);
    }
    else if (cmd == "UPDATE")
    {
      db->update(kv_pair);
    }
    else
    {
      cout << "[ERROR] INVALID COMMAND: " << cmd << endl;
    }
    cmdNum++;
    if (cmdNum % 10000 == 0)
      cout << "[STATUS] has run " << cmdNum << " command" << endl;
  }
  time_t end = clock();
  cout << "[STATUS] Init DB finished, use time " << ((double)end - start) / CLOCKS_PER_SEC << "ms" << endl
       << endl
       << endl;
  f.close();
}

int main()
{
  getFileList();
  int test1 = 12345670;
  // cout << stringTo64("85264696") << endl;
  for (int i = 0; i < testNum; i++)
  {
    // PmEHash db;
    loadFile(loadFileList[i]);
    runFile(runFileList[i]);
  }
}
