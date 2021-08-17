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
from machine import Pin
from machine import I2C

DEMO_VERSION = '21053101'


data_trans_ready = False
gpio_toggle_flag = False

def get_battery_vol():
    """
    获取电池电压，单位mV
    """
    return misc.Power.getVbatt()


class aht10class():
    i2c_dev = None
    i2c_addre = None

    # Initialization command
    AHT10_CALIBRATION_CMD = 0xE1
    # Trigger measurement
    AHT10_START_MEASURMENT_CMD = 0xAC
    # reset
    AHT10_RESET_CMD = 0xBA

    def write_data(self, data):
        self.i2c_dev.write(self.i2c_addre,
                           bytearray(0x00), 0,
                           bytearray(data), len(data))
        pass

    def read_data(self, length):
        r_data = [0x00 for i in range(length)]
        r_data = bytearray(r_data)
        self.i2c_dev.read(self.i2c_addre,
                          bytearray(0x00), 0,
                          r_data, length,
                          0)
        return list(r_data)

    def aht10_init(self, addre=0x38):
        self.i2c_dev = I2C(I2C.I2C0, I2C.STANDARD_MODE)  # 返回i2c对象
        self.i2c_addre = addre
        self.sensor_init()
        pass

    def aht10_transformation_temperature(self, data):
        r_data = data
        #　根据数据手册的描述来转化温度
        humidity = (r_data[0] << 12) | (
            r_data[1] << 4) | ((r_data[2] & 0xF0) >> 4)
        humidity = (humidity/(1 << 20)) * 100.0
        print("current humidity is {0}%".format(humidity))
        temperature = ((r_data[2] & 0xf) << 16) | (
            r_data[3] << 8) | r_data[4]
        temperature = (temperature * 200.0 / (1 << 20)) - 50
        print("current temperature is {0}°C".format(temperature))
        return humidity, temperature
        

    def sensor_init(self):
        # calibration
        self.write_data([self.AHT10_CALIBRATION_CMD, 0x08, 0x00])
        time.sleep_ms(300)  # at last 300ms
        pass

    def ath10_reset(self):
        self.write_data([self.AHT10_RESET_CMD])
        time.sleep_ms(20)  # at last 20ms

    def trigger_measurement(self):
        # Trigger data conversion
        self.write_data([self.AHT10_START_MEASURMENT_CMD, 0x33, 0x00])
        time.sleep_ms(200)  # at last delay 75ms
        # check has success
        r_data = self.read_data(6)
        # check bit7
        if (r_data[0] >> 7) != 0x0:
            print("Conversion has error")
            return None
        else:
            return self.aht10_transformation_temperature(r_data[1:6])


def i2c_aht10_test():
    ath_dev = aht10class()
    ath_dev.aht10_init()

    # 测试十次
    for i in range(10):
        ath_dev.Trigger_measurement()
        time.sleep(1)


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
        """ 每十秒调用一次回调函数 """ 
        ostimer.start(10000, 1, self.mainTask)

        """" 创建gpio对象 """
        self.gpio1 = Pin(Pin.GPIO1, Pin.OUT, Pin.PULL_DISABLE, 0)

        """ 初始化aht10 """
        self.ath_dev = aht10class()
        self.ath_dev.aht10_init()

        """ 设置自动休眠模式 """
        pm.autosleep(1)

        """ 启动云平台连接 """
        quecIot.setConnmode(1)

    @staticmethod
    def eventCB(data):
        print("\r\n{},{}\r\n".format(str(data[0]), str(data[1])))
        if len(data) == 3:
            print(data[2])
            """
            测试发送物模型数据
            向平台发送数据需要在返回3, 10200之后进行
            """
        if 1 == data[0] and 10422 == data[1]:
            quecIot.setConnmode(0)
            exit(0)
        elif 3 == data[0] and 10200 == data[1]:
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

            temperature = self.ath_dev.trigger_measurement()
            """ 发送物模型数据 """
            quecIot.phymodelReport(0, {110 : battery_vol})
            quecIot.phymodelReport(0, {111 : temperature})

            """ 翻转引脚电平 """
            # gpio_toggle_flag = not gpio_toggle_flag
            # self.gpio1.write(int(gpio_toggle_flag))
        
            

if __name__ == '__main__':
    print('\r\n**********\r\n')
    print(DEMO_VERSION)
    quecthing = Quecthing()
