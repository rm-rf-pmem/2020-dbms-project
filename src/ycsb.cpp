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

  for (int i = 0; i < loadFileList.size(); i++)
  {
    files.push_back(*new txtInfo(loadFileList[i]));
    cout << "[FIND FILE] " << files[i].getLoadName() << ' ' 
         << files[i].getRunName() << endl;
  }

  cout << endl << endl;

  testNum = loadFileList.size(); // 文件数量
}

void loadFile(txtInfo file, PmEHash *db = nullptr)
{
  fstream f;
  f.open(WORKLOAD + file.getLoadName(), ios::in);

  // 开始标识栏
  cout << "[STATUS] start to init DB with " << file.getLoadName() << endl;
  cout << "---------------------------------------------------" << endl;
  string cmd = "";
  string data = "";
  time_t start = clock(); // 计时开始
  long long dataNum = 0; // 计算数据数量

  while (!f.eof())
  {
    cmd = data = "";
    f >> cmd;
    if (cmd == "")
      break;
    f >> data;

    // 构建 kv 对
    string k = data.substr(0, 8);
    uint64_t v = atoi(data.substr(8).c_str());
    kv kv_pair;
    // kv_pair.key = rand();
    // kv_pair.value = rand();
    kv_pair.key = stringTo64(k);
    kv_pair.value = v;

    // 测试时注释掉
    db->insert(kv_pair); //插入操作

    dataNum++;
    // if (dataNum % 100000 == 0) // 每1W次操作就进行一次输出
    //   cout << "[STATUS] has load " << dataNum << "/" << file.getDataSize() << " data" << endl;
  }

  time_t end = clock(); // 计时结束

  // 结束标识栏
  cout << "[STATUS] Init DB finished, use time ";
  printf("%.5lf", ((double)end - start) / CLOCKS_PER_SEC); // 输出操作时间
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
      cout << "[ERROR] INVALID COMMAND: " << cmd << endl;
    }

    // 好像没见到删除

    cmdNum++;
    // if (cmdNum % 1000000 == 0) // 每1W次操作删除一次
    //   cout << "[STATUS] has run " << cmdNum << " command" << endl;
  }
  time_t end = clock(); // finish run

  // 结束标识栏
  double spentTime = ((double)end - start) / CLOCKS_PER_SEC;
  cout << "[STATUS] Run DB finished "<< endl;
  cout << "---------------------------------------------------" << endl;

  cout << "use time ";
  printf("%.4lfs\n", spentTime);                                    // 操作时间
  cout << "r/w = " << read << '/' << write << endl                  // 读写比
       << "speed: " << (long)(cmdNum / spentTime) << " commands/s"  // 操作数/秒
       << endl << endl << endl;
  f.close();
}

int main()
{
  // 调试用的
  getFileList();
  // PmEHash *db = new PmEHash();
  // loadFile(files[5], db);
  // runFile(files[5], db);
  // db->selfDestory();

  for (int i = 0; i < testNum; i++)
  {
    PmEHash *db = new PmEHash();
    loadFile(files[i], db);
    runFile(files[i], db);
    db->selfDestory();
  }
}
