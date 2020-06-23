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
#include <cstdlib>

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
  for (int i = 0; i < str.length() && i < 8; i++)
  {
    uint8_t temp = str[i];
    res = res << 8; // 左移8位
    res += temp;
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
  if ((dp = opendir(WORKLOAD)) == NULL) // 打开文件夹失败
  {
    cout << "Can't open " << WORKLOAD << endl;
  }
  while ((dirp = readdir(dp)) != NULL) // 打开文件夹
  {
    if (regex_match(dirp->d_name, loadReg))
      loadFileList.push_back(dirp->d_name);
  }
  closedir(dp);

  // 排序
  sort(loadFileList.begin(), loadFileList.end());

  // 循环输出 load-run 文件对
  for (int i = 0; i < loadFileList.size(); i++)
  {
    files.push_back(*new txtInfo(loadFileList[i]));
    printf("[FIND FILE] %s %s\n", 
            files[i].getLoadName().c_str(), 
            files[i].getRunName().c_str());
  }

  printf("\n\n\n");

  testNum = loadFileList.size(); // 文件数量
}

/*
 * 加载 load.txt 文件
 * 对数据库进行初始化
*/
void loadFile(txtInfo file, PmEHash *db = nullptr)
{
  fstream f;
  f.open(WORKLOAD + file.getLoadName(), ios::in);

  // 开始标识栏
  printf("[STATUS] start to init DB with %s\n", file.getLoadName().c_str());
  printf("---------------------------------------------------\n");

  string cmd = "";
  string data = "";
  time_t start = clock(); // 计时开始
  long long dataNum = 0; // 计算数据数量

  while (!f.eof())
  {
    cmd = data = ""; // 清空数据
    f >> cmd;
    if (cmd == "") // 终止输入，避免报错
      break;
    f >> data;

    // 构建 kv 对
    string k = data.substr(0, 8);
    uint64_t v = atoi(data.substr(8).c_str());
    kv kv_pair;
    kv_pair.key = stringTo64(k);
    kv_pair.value = v;

    // 测试时注释掉
    db->insert(kv_pair); //插入操作

    dataNum++;
    if (dataNum % 100000 == 0) // 每10W次操作就进行一次输出
      printf("[STATUS] has load %ld/%ld data\n", dataNum, file.getDataSize());
  }

  time_t end = clock(); // 计时结束

  // 结束标识栏
  printf("[STATUS] Init DB finished, use time "
         "%.5lf s\n\n\n", ((double)end - start) / CLOCKS_PER_SEC); // 输出操作时间
  f.close();
}

/*
 * 运行 run.txt 文件
*/
void runFile(txtInfo file, PmEHash *db = nullptr)
{
  fstream f;
  f.open(WORKLOAD + file.getRunName(), ios::in);

  // 开始标志栏
  printf("[STATUS] start to run DB with %s\n", file.getRunName().c_str());
  printf("---------------------------------------------------\n");

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
    if (cmd == "READ") // 对应搜索
    {
      db->search(kv_pair.key, res);
      read++;
    }
    else if (cmd == "INSERT") // 对应插入
    {
      db->insert(kv_pair);
      write++;
    }
    else if (cmd == "UPDATE") // 对应更新
    {
      db->update(kv_pair);
      write++;
    }
    else
    {
      printf("[ERROR] INVALID COMMAND: %s\n", cmd);
    }

    // 好像没见到删除

    cmdNum++;
    if (cmdNum % 100000 == 0) // 每10W次操作删除一次
      printf("[STATUS] has run %ld operations\n", cmdNum);
  }
  time_t end = clock(); // finish run

  double spentTime = ((double)end - start) / CLOCKS_PER_SEC; // 经过时间

  // 结束标识栏
  printf("[STATUS] Run DB finished \n");
  printf("---------------------------------------------------\n");

  printf("operation size: %ld\n", cmdNum);                               // 总操作数
  printf("use time %.4lfs\n", spentTime);                               // 操作时间
  printf("r/w = %d/%d\n", read, write);                                 // 读写比
  printf("OPS: %ld operations/s\n\n\n", (long)(cmdNum / spentTime));    // 每秒操作数
  f.close();
}

int main()
{
  getFileList(); // 获取文件列表

  // 对个 load-run 文件对进行测试
  for (int i = 0; i < testNum; i++)
  {
    PmEHash *db = new PmEHash();
    loadFile(files[i], db);
    runFile(files[i], db);
    db->selfDestory();
  }
}
