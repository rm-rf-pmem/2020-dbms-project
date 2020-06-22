# 编译加了libpmem库的源文件时加上-l pmem参数进行编译
# 在这个makefile中添加ycsb测试的相关编译命令，运行ycsb

CC=g++
INCDIR=../include
src=$(shell find ./ -name "*cpp")
deps=$(shell find ${INCDIR} -name "*.h")
obj=$(src: %.c-%.o)

ycsb:	ycsb.o pm_ehash.o data_page.o
	${CC} -o $@ 

%.o: %.cpp ${deps}
	${CC} -c $< -o $@

clean: 
	@rm *.o
	@rm ycsb