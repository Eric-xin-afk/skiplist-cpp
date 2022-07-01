# 1.项目介绍：基于跳表实现的轻量级键值型存储引擎(K-V存储引擎)
    非关系型数据库redis,leveldb,rockdb等其核心存储引擎的数据结构就是跳表，本项目就是基于跳表实现的轻量级键值型存储引擎，使用C++实现。在随机读写情况下，该项目QPS(每秒可处理写请求数)：10万，读请求数：6万。

# 2.功能实现 -- 提供的对应接口
    插入数据            -- insertElement
    删除数据            -- deleteElement
    查询数据            -- searchElement
    数据显示            -- displayList
    数据落盘            -- dumpFile  
    文件加载数据         -- loadFile 
    数据库大小显示(数据规模) -- size

# 3.项目组织结构
    main.cpp -- 包含skiplist.h使用跳表进行数据操作
    skip.h -- 跳表核心实现
    README.md -- 中文介绍
    bin -- 生成可执行文件目录
    makefile -- 编辑脚本
    store -- 数据落盘的文件放在此文件夹
    stress_test_start.sh -- 压力测试脚本
    LICENCE使用协议

# 4.存储引擎数据表现
    插入操作(随即插入数据测试)：10万/秒
    取数据操作：每秒可处理请求数：6万/秒

# 5.项目部署方式
    make
    ./bin/main

    脚本测试：
        sh stress_test_start.sh

# 6.待优化
    互斥量的优化mutex, lock() --> lock_guard()
    delete的时候没有释放内存：链表节点没有释放-->使用指针指向，然后释放内存空间
    压力测试并不是全自动的
    跳表的key用int型，如果使用其他类型需要自定义比较函数，当然把这块抽象出来更好
    如果再加上一致性协议，例如raft就构成了分布式存储，再启动一个http server就可以对外提供分布式存储服务了(待实现)

注：参考 https://github.com/youngyangyang04/Skiplist-CPP
