#ifndef INFO
#define INFO
#include <iostream>
using namespace std;

class txtInfo
{
private:
  string filename;
  long dataSize;
  int read;
  int write;

public:
  txtInfo(string loadfilename);

  /**
   * 获取 loat.txt 格式的文件名
  */
  string getLoadName();

  /**
   * 获取 run.txt 格式的文件名
  */
  string getRunName();

  /**
   * 获取数据量大小
  */
  long getDataSize();

  /**
   * 获取读比例
  */
  int getRead();

  /**
   * 获取写比例
  */
  int getWrite();
};

#endif