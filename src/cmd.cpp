#include "cmd.h"
#include <string.h>
#include <iostream>

txtInfo::txtInfo(string loadfilename)
{
  string name = loadfilename.substr(0, loadfilename.length() - 8);
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
string txtInfo::getLoadName()
{
  return this->filename + "load.txt";
}

/**
   * 获取 run.txt 格式的文件名
  */
string txtInfo::getRunName()
{
  return this->filename + "run.txt";
}

/**
   * 获取数据量大小
  */
long txtInfo::getDataSize()
{
  return this->dataSize;
}

/**
   * 获取读比例
  */
int txtInfo::getRead()
{
  return this->read;
}

/**
   * 获取写比例
  */
int txtInfo::getWrite()
{
  return this->write;
}
