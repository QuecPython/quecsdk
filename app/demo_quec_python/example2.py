# -*- coding: utf-8 -*-
# @Time    : 2021-03-26
# @Author  : evan.li
# @File    : main.py
# @Brief   : quecthing
# @revise  :
# 2021-05-26 add get_battery_vol

import quecIot


class Quecthing:
    def __init__(self):
        """ 初始化qucsdk """
        quecIot.init()
        """ 注册事件回调函数 """
        quecIot.setEventCB(self.eventCB)
        """ 配置产品信息"""
        quecIot.setProductinfo("p1115X", "d2c5Q1FsVWpwT1k3")
        """ 配置服务器信息，可选，默认连接MQTT生产环境服务器 """
        quecIot.setServer(1,"http://iot-south.quectel.com:2883")
        """ 配置lifetime，可选，MQTT默认为120 """
        quecIot.setLifetime(120)
        """ 配置外部MCU标识号和版本号，可选，如没有外部MCU则不需要配置 """
        quecIot.setMcuVersion("MCU1", "1_0_0")
        """ 启动云平台连接 """
        quecIot.setConnmode(1)

    @staticmethod
    def eventCB(data):
        print(str(data[0]) + "," + str(data[1]) + "\r\n")
        if len(data) == 3:
            print(data[2])
            """
            测试发送物模型数据
            向平台发送数据需要在返回3, 10200之后进行
            """
        if 3 == data[0] and 10200 == data[1]:
            """ 发送bool型数据"""
            quecIot.phymodelReport(1, {2: True})
            """ 发送数值 """
            # 整数
            quecIot.phymodelReport(1, {3: 123})
            # 浮点数
            quecIot.phymodelReport(1, {9: 123.123})
            """ 发送array """
            quecIot.phymodelReport(1, {4: [1, 2, 3]})
            """ 发送结构体 """
            quecIot.phymodelReport(1, {6: {8: 1.0, 7: 1.0}})

if __name__ == '__main__':
    Quecthing()
