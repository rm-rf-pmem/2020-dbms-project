#include "pm_ehash.h"
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

class Infor
{
private:
  string filename;
  long dataSize;
  int read;
  int write;

public:
  Infor(string loadfilename)
  {
    string name = loadfilename.substr(0, loadfilename.length() - 9);
    this->filename = name;
    int flag = name.find("-", 0);                                // 找到第一个 '-'，第一个为数据总量
    this->dataSize = atoi(name.substr(0, flag).c_str()) * 10000; // 获取数据总量
    name = name.substr(flag + 1, name.length() - 1);             // 第一次截断
    flag = name.find("-", 0);                                    // 找到第二个 '-'，第二个为权限类型
    name = name.substr(flag + 1, name.length() - 1);             // 第二次截断
    flag = name.find("-", 0);                                    // 找到第三个 '-'，第三个为读取比例
    this->read = atoi(name.substr(0, flag).c_str());             // 获取读取比例
    this->write = 100 - this->read;                              //计算得到写比例
  }

  /**
   * 获取 loat.txt 格式的文件名
  */
  string getLoadName()
  {
    return this->filename + "load.txt";
  }

  /**
  *  获取 run.txt 的文件名
  */
  string getRunName()
  {
    return this->filename + "run.txt";
  }

  /**
   * 获取数据量大小
  */
  long getDataSize()
  {
    return this->dataSize;
  }

  /**
   * 获取读比例
  */
  int getRead()
  {
    return this->read;
  }

  /**
   * 获取写比例
  */
  int getWrite()
  {
    return this->write;
  }
};

// 假定每一个 load 都有一个 run 对应
vector<string> loadFileList;                              // 储存对应文件
int testNum;                                              // 储存 load-run 文件对数量
const regex loadReg("(\\d+)w-rw-(\\d+)-(\\d+)-load.txt"); // load.txt 的正则匹配
vector<Infor> files;                                      // 储存文件相关信息

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
    files.push_back(*new Infor(loadFileList[i]));
    cout << "[FIND FILE] " << files[i].getLoadName() << ' ' << files[i].getRunName() << endl;
  }

  cout << endl
       << endl;
  testNum = loadFileList.size();
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
    if (cmd == "")
      break;
    f >> data;
    string k = data.substr(0, 8);
    int v = atoi(data.substr(8).c_str());
    kv kv_pair;
    kv_pair.key = stringTo64(k);
    kv_pair.value = v;
    uint64_t res = 0;
    // 暂时注释掉
    // if (cmd == "READ")
    // {
    //   db->search(kv_pair.key, res);
    // }
    // else if (cmd == "INSERT")
    // {
    //   db->insert(kv_pair);
    // }
    // else if (cmd == "UPDATE")
    // {
    //   db->update(kv_pair);
    // }
    // else
    // {
    //   cout << "[ERROR] INVALID COMMAND: " << cmd << endl;
    // }
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
  }
}
