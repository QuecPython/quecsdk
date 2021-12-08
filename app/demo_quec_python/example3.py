# -*- coding: utf-8 -*-
# @Time    : 2021-03-26
# @Author  : evan.li
# @File    : main.py
# @Brief   : quecthing
# @revise  :
# 2021-05-26 add get_battery_vol

import quecIot
from machine import Timer

class Quecthing:
    def __init__(self):
        """ 初始化qucsdk """
        quecIot.init()
        """ 注册事件回调函数 """
        quecIot.setEventCB(self.eventCB)
        """ 配置产品信息"""
        quecIot.setProductinfo("p111HL", "WVF1aHYyMlVXVUl4")
        """ 配置服务器信息，可选，默认连接MQTT生产环境服务器 """
        quecIot.setServer(1,"http://iot-south.quectel.com:2883")
        """ 配置lifetime，可选，MQTT默认为120 """
        quecIot.setLifetime(120)
        """ 配置外部MCU标识号和版本号，可选，如没有外部MCU则不需要配置 """
        quecIot.setMcuVersion("MCU1", "1_0_0")
        """ 启动云平台连接 """
        quecIot.setConnmode(1)

        """ 配置子设备回调函数 """
        quecIot.subDevSetEventCB(subDevEventCB)
        
    def subDev_PassTransMode(self):
        """ 子设备发起认证到平台 """
        quecIot.subDevConn("p111HM", "VzY3dGo2UEF5eDE5", "8EEC4B66AEE8", 0, 120)
        """ 子设备登陆到平台 """
        """ 若子设备已认证到平台，之后调用登陆接口时需要将认证得到的ds信息放到方法中；如下：
        self.__ds = "1234"
        quecIot.subDevConn("p111HM", "VzY3dGo2UEF5eDE5", "8EEC4B66AEE8", self.__ds, 0, 120)
        """
    def passTranDev_recvDs(self,ds):
        self.__ptDs = ds
        print("device id 8EEC4B66AEE8 product key: p111HM, receive ds:"+str(ds))

    def passTranDev_timerCB(self):
        quecIot.subDevHTB("p111HM", "8EEC4B66AEE8")
        ptTimer.stop()
        ptTimer.start(period=60 * 1000,mode=Timer.PERIODIC, callback=self.passTranDev_timerCB)

    def passTranDev_connSuccess(self):
        quecIot.subDevPassTransSend("p111HM", "8EEC4B66AEE8", "123456")
        ptTimer = Timer(Timer.ptTimer)
        ptTimer.start(period=60 * 1000,mode=Timer.PERIODIC, callback=self.passTranDev_timerCB)
    
    def passTranDev_timerStop(self):
        ptTimer.stop()

    def subDev_TslMode(self):
        """ 子设备发起认证到平台 """
        quecIot.subDevConn("p111HN", "Vm9pcmR2Mzd4cXB0", "8EEC4B66AEE9", 0, 120)
        """ 子设备登陆到平台 """
        """ 若子设备已认证到平台，之后调用登陆接口时需要将认证得到的ds信息放到方法中；如下：
        self.__ds = "1234"
        quecIot.subDevConn("p111HN", "Vm9pcmR2Mzd4cXB0", "8EEC4B66AEE9", self.__ds, 0, 120)
        """

    def tslDev_recvDs(self,ds):
        self.__tslDs = ds
        print("device id 8EEC4B66AEE9 product key: p111HN, receive ds:"+str(ds))

    def tslDev_timerCB(self):
        quecIot.subDevHTB("p111HM", "8EEC4B66AEE8")
        tslTimer.stop()
        tslTimer.start(period=60 * 1000,mode=Timer.PERIODIC, callback=self.tslDev_timerCB)

    def tslDev_timerStop(self):
        tslTimer.stop()

    def tslDev_connSuccess(self):
        """ 发送bool型数据"""
        quecIot.subDevTslReport("p111HN", "8EEC4B66AEE9", {2: True})
        """ 发送数值 """
        # 整数
        quecIot.subDevTslReport("p111HN", "8EEC4B66AEE9", {3: 123})
        # 浮点数
        quecIot.subDevTslReport("p111HN", "8EEC4B66AEE9", {9: 123.123})
        """ 发送array """
        quecIot.subDevTslReport("p111HN", "8EEC4B66AEE9", {4: [1, 2, 3]})
        """ 发送结构体 """
        quecIot.subDevTslReport("p111HN", "8EEC4B66AEE9", {6: {8: 1.0, 7: 1.0}})
        tslTimer = Timer(Timer.tslTimer)
        tslTimer.start(period=60 * 1000,mode=Timer.PERIODIC, callback=self.tslDev_timerCB)


    def subDevEventCB(self,data):
        print(data[0]+","+data[1]+","+str(data[2])+","+str(data[3]))
        if len(data) == 5:
            print(data[4])
        if 1 == data[2] and 10200 == data[3]:
            if 5 == len(data):
                if "p111HM" == data[0]and "8EEC4B66AEE8" == data[1]:
                    self.passTranDev_recvDs(data[4])
                elif "p111HN" == data[0] and "8EEC4B66AEE9" == data[1]:
                    self.tslDev_recvDs(data[4])
            else:
                print("register platform error:" + data[3])

        if 2 == data[2] and 10200 == data[3]:
            if "p111HM" == data[0]and "8EEC4B66AEE8" == data[1]:
                self.passTranDev_connSuccess()
            elif "p111HN" == data[0] and "8EEC4B66AEE9" == data[1]:
                self.tslDev_connSuccess()
        
        if 6 == data[2] and 10200 == data[3]:
            if "p111HM" == data[0]and "8EEC4B66AEE8" == data[1]:
                self.passTranDev_timerStop()
            elif "p111HN" == data[0] and "8EEC4B66AEE9" == data[1]:
                self.tslDev_timerStop()


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
            """ 透传子设备 """
            Quecthing.subDev_PassTransMode()
            """" 物模型子设备 """
            Quecthing.subDev_TslMode()

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
