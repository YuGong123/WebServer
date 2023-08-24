CXX = g++
CFLAGS = -std=c++14 -O2 -Wall -g 
# -std=c++14，采用c++14 标准
# -O 2优化级别为2
# -Wall 生成所有警告信息
# -g 在编译的时候，生成调试信息，该程序可以被调试器调试


# 生成目标，可执行程序
TARGET = server 	

# 生成目标的依赖
OBJS = ../code/log/*.cpp ../code/pool/*.cpp ../code/timer/*.cpp \
       ../code/http/*.cpp ../code/server/*.cpp \
       ../code/buffer/*.cpp ../code/main.cpp

all: $(OBJS)
	$(CXX) $(CFLAGS) $(OBJS) -o ../bin/$(TARGET)  -pthread -lmysqlclient
# 线程库，mysql-client库

clean:
	rm -rf ../bin/$(OBJS) $(TARGET)
# 删除./bin/server 和依赖



