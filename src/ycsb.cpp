#include "pm_ehash.h"
#include "cmd.h"
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

// 假定每一个 load 都有一个 run 对应
vector<string> loadFileList;                              // 储存对应文件
int testNum;                                              // 储存 load-run 文件对数量
const regex loadReg("(\\d+)w-rw-(\\d+)-(\\d+)-load.txt"); // load.txt 的正则匹配
vector<txtInfo> files;                                      // 储存文件相关信息

/**
  * @将前8位字符转换位 uint64_t
*/
uint64_t stringTo64(string str)
{
  uint64_t res = 0;
  for (int i = 0; i < str.length(); i++)
  {
    uint8_t temp = str[i];
    res = res << 8; // 左移8位
    res += temp;
    // printf("%d\t%d\t%ld\n", str[i], temp, res);
  }

  return res;
}

/**
 * @获取文件夹中的文件
*/
void getFileList()
{
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
  }
  closedir(dp);

  // 排序
  sort(loadFileList.begin(), loadFileList.end());

  for (int i = 0; i < loadFileList.size(); i++)
  {
    files.push_back(*new txtInfo(loadFileList[i]));
    cout << "[FIND FILE] " << files[i].getLoadName() << ' ' << files[i].getRunName() << endl;
  }

  cout << endl
       << endl;
  testNum = loadFileList.size();
}

void loadFile(txtInfo file, PmEHash *db = nullptr)
{
  fstream f;
  f.open(WORKLOAD + file.getLoadName(), ios::in);
  cout << "[STATUS] start to init DB with " << file.getLoadName() << endl;
  cout << "---------------------------------------------------" << endl;
  string cmd = "";
  string data = "";
  time_t start = clock();
  long long dataNum = 0;
  while (!f.eof())
  {
    cmd = data = "";
    f >> cmd;
    if (cmd == "")
      break;
    f >> data;
    string k = data.substr(0, 8);
    int v = atoi(data.substr(8).c_str());
    kv kv_pair;
    kv_pair.key = stringTo64(k);
    kv_pair.value = v;
    // 测试时注释掉
    db->insert(kv_pair);
    dataNum++;
    if (dataNum % 10000 == 0)
      cout << "[STATUS] has load " << dataNum << "/" << file.getDataSize() << " data" << endl;
  }
  time_t end = clock();
  cout << "[STATUS] Init DB finished, use time ";
  printf("%.4lf", ((double)end - start) / CLOCKS_PER_SEC);
  cout << "s" << endl << endl << endl;
  f.close();
}

void runFile(txtInfo file, PmEHash *db = nullptr)
{
  fstream f;
  f.open(WORKLOAD + file.getRunName(), ios::in);

  // 开始标志栏
  cout << "[STATUS] start to run DB with " << file.getRunName() << endl;
  cout << "---------------------------------------------------" << endl;
  string cmd = "";
  string data = "";
  time_t start = clock(); // 计时开始
  long long cmdNum = 0;   // 记录已经加载的命令数量
  long long read = 0;     // 记录读操作
  long long write = 0;    // 记录写操作

  while (!f.eof())
  {
    cmd = data = ""; // 清空数据
    f >> cmd;
    if (cmd == "") // 终止输入，避免报错
      break;
    f >> data;

    // 获取 k 和 v
    string k = data.substr(0, 8);
    int v = atoi(data.substr(8).c_str());

    // 建立新的 kv 对
    kv kv_pair;
    kv_pair.key = stringTo64(k);
    kv_pair.value = v;

    // 用于储存查找的结果
    uint64_t res = 0;

    // 暂时注释掉
    if (cmd == "READ")
    {
      db->search(kv_pair.key, res);
      read++;
    }
    else if (cmd == "INSERT")
    {
      db->insert(kv_pair);
      write++;
    }
    else if (cmd == "UPDATE")
    {
      db->update(kv_pair);
      write++;
    }
    else
    {
      cout << "[ERROR] INVALID COMMAND: " << cmd << endl;
    }
    cmdNum++;
    if (cmdNum % 10000 == 0)
      cout << "[STATUS] has run " << cmdNum << " command" << endl;
  }
  time_t end = clock(); // finish run


  double spentTime = ((double)end - start) / CLOCKS_PER_SEC;
  cout << "[STATUS] Run DB finished "<< endl;
  cout << "---------------------------------------------------" << endl;

  cout << "use time ";
  printf("%.4lfs\n", spentTime);
  cout << "r/w = " << read << '/' << write << endl
       << "speed: " << (long)(cmdNum / spentTime) << " commands/s"
       << endl << endl << endl;
  f.close();
}

int main()
{
  getFileList();
  int test1 = 12345670;
  // cout << stringTo64("85264696") << endl;
  for (int i = 0; i < testNum; i++)
  {
    PmEHash *db = new PmEHash();
    loadFile(files[i], db);
    runFile(files[i], db);
  }
}
