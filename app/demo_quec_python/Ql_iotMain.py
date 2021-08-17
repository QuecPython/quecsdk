# Copyright 2020 - 2021 quectel
# -*- coding: utf-8 -*-
# @Time    : 2021-03-26
# @Author  : evan.li
# @File    : Ql_iotMain.py
# @Brief   : quecthing
# @revise  :
# 2021-05-26 add get_battery_vol

import quecIot
import misc
import utime as time
import pm
import osTimer
import uos
from machine import Pin

DEMO_VERSION = '21060101'


data_trans_ready = False
gpio_toggle_flag = False

def get_battery_vol():
    """
    获取电池电压，单位mV
    """
    return misc.Power.getVbatt()

"""
    模组OTA升级和python 脚本升级
"""
def ota_event(event):
    if 7 == event[0] and 10700 == event[1]:
        """ 检查组件标志, 组件标志在平台创建 """
        if "SCRIPT" in event[2]:
            quecIot.otaAction(1)
        elif "IMEI" in event[2]:
            quecIot.otaAction(1)
    elif 7 == event[0] and 10701 == event[1]:
        pass
    elif 7 == event[0] and 10703 == event[1]:
        if "SCRIPT" in event[2]:
            
            uos.rename('usr/demo_quecthing.py', 'usr/demo_quecthing.bk')
            uos.rename('usr/qiot_ota.bin', 'usr/demo_quecthing.py')

            """ 设置版本, 上报升级成功"""
            quecIot.setMcuVersion('SCRIPT', 'v2')
        elif "IMEI" in event[2]:
            pass


class Quecthing:
    def __init__(self):
        """ 初始化qucsdk """
        quecIot.init()
        """ 注册事件回调函数 """
        quecIot.setEventCB(self.eventCB)
        """ 配置产品信息"""
        quecIot.setProductinfo("p1116a", "UHg1dTRBRVh3MkVG")
        """ 配置服务器信息，可选，默认连接MQTT生产环境服务器 """
        quecIot.setServer(1,"http://iot-south.quectel.com:2883")
        """ 配置lifetime，可选，MQTT默认为120 """
        quecIot.setLifetime(120)
        """ 配置外部MCU标识号和版本号，可选，如没有外部MCU则不需要配置 """
        quecIot.setMcuVersion("MCU1", "1_0_0")   

        """ 创建定时器任务 """ 
        ostimer = osTimer()
        """ 每一百秒调用一次回调函数 """ 
        ostimer.start(100000, 1, self.mainTask)

        """" 创建gpio对象 """
        self.gpio1 = Pin(Pin.GPIO1, Pin.OUT, Pin.PULL_DISABLE, 0)


        """ 设置自动休眠模式 """
        pm.autosleep(1)

        """ 启动云平台连接 """
        quecIot.setConnmode(1)

    @staticmethod
    def eventCB(data):
        print("\r\n{},{}\r\n".format(str(data[0]), str(data[1])))
        if len(data) == 3:
            print(data[2])
            
        ota_event(data)
        if 1 == data[0] and 10422 == data[1]:
            quecIot.setConnmode(0)
            exit(0)
        elif 3 == data[0] and 10200 == data[1]:
            """
            测试发送物模型数据
            向平台发送数据需要在返回3, 10200之后进行
            """
            global data_trans_ready
            data_trans_ready = True
            

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

            """ 发送中文,透传数据和物模型数据无法同时在平台调试 """

            """
                bytes_temp = bytes('中文'.encode('utf-8'))
                quecIot.passTransSend(1, bytes_temp)
            """

    def mainTask(self, argv):
        global data_trans_ready
        global gpio_toggle_flag
        """
        主任务,上报电池电压
        """
        if data_trans_ready:
            print('main task')
            """ 获取电压 """
            battery_vol = get_battery_vol()

            """ 发送物模型数据 """
            quecIot.phymodelReport(0, {110 : battery_vol})

            """ 翻转引脚电平 """
            # gpio_toggle_flag = not gpio_toggle_flag
            # self.gpio1.write(int(gpio_toggle_flag))
        
            

if __name__ == '__main__':
    print('\r\n**********\r\n')
    print(DEMO_VERSION)
    quecthing = Quecthing()
