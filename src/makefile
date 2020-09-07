# 编译加了libpmem库的源文件时加上-l pmem参数进行编译
# 在这个makefile中添加ycsb测试的相关编译命令，运行ycsb

CC := g++
INCDIR := ../include
FLAGS := -std=c++11 -w

ycsb:	ycsb.o pm_ehash.o data_page.o cmd.o
	${CC} ${FLAGS} -o $@ $^ -lpmem 
	sudo ./ycsb

%.o: %.cpp
	${CC} ${FLAGS} -I ${INCDIR} -o $@ -c $^

clean: 
	@rm *.o

cleand:
	@rm -r data