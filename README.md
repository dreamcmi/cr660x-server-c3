## CR660X-SERVER-C3

## 一、简介

- 用于给小米cr660x系列路由器解锁ssh，原方案需要路由器辅助，操作难度大，故有了此项目。

- 本质就是访问一个http，所以这里用esp32-c3做了一个简易的server。

- 至于为啥选esp32c3，第一是esp32c3模组价格和esp8266模组差不多(只针对平常售价，如某某可)，第二是本人对于ESP-IDF较为熟悉。
- C语言代码一共100行不到，原理简单，有能力的朋友可以自行使用其他方法解决。

## 二、使用

### (1)使用编译好的bin文件

本仓库bin文件夹下有编译好的esp32c3固件，直接使用esptool工具刷到0x0地址即可使用

### (2)自行编译

1. 先安装ESP-IDF环境，推荐v4.4
2. 进入本仓库执行

```shell
idf.py set-target esp32c3
idf.py build
idf.py -p COM3 flash
```

## 三、注意事项

默认配置

AP名称：C3CR660X 

密码:12345678 

网关:169.254.31.1

掩码:255.255.0.0

## 四、LICENSE

GPL V2.0

未经本人授权禁止任何形式商用！！！