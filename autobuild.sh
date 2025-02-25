#!/bin/bash

set -e

# 如果没有build目录，创建该目录
if [ ! -d `pwd`/build ]; then
    mkdir `pwd`/build
fi

rm -rf `pwd`/build/*

cd `pwd`/build &&
    cmake .. &&
    make

# 回到项目根目录
cd ..

# 把头文件拷贝到 /usr/include/hnreactor  so库拷贝到 /usr/lib    PATH
if [ ! -d /usr/include/hnreactor ]; then 
    mkdir /usr/include/hnreactor
fi

#循环:把当前目录的`ls *.h`文件拷贝到/usr/include/hnreactor
for header in `ls *.h` #当前目录的所有头文件
do
    cp $header /usr/include/hnreactor
done

cp `pwd`/lib/libhnreactor.so /usr/lib

#有时候动态库已经放入了.但还是找不到,就要刷新一下
ldconfig