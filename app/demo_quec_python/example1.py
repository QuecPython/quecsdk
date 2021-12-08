# 设置GPIO高电平并设置定时器指定时长后拉低GPIO

import quecIot
from machine import Timer
from machine import Pin

timer1 = None
TimerCB = None
Gpio_relay = None
remaining_time =0

class Quecthing:

    def __init__(self):
        global Gpio_relay
        global timer1
        global TimerCB

        """ 初始化qucsdk """
        quecIot.init()
        """ 注册事件回调函数 """
        quecIot.setEventCB(self.eventCB)
        """ 配置产品信息"""
        quecIot.setProductinfo("p1117y", "OFZCcWt1TjVUeUdn")
        """ 启动云平台连接 """
        quecIot.setConnmode(1)
        timer1 = Timer(Timer.Timer1)
        TimerCB = self.timerTask
        Gpio_relay = Pin(Pin.GPIO10, Pin.OUT, Pin.PULL_DISABLE, 0)

    @staticmethod
    def eventCB(data):
        global Gpio_relay
        global timer1
        global TimerCB
        global remaining_time

        print(str(data[0]) + "," + str(data[1]))
        if len(data) == 3:
            print(data[2])

        if 5 == data[0] and 10210 == data[1]:
            model = data[2]
            print(model.keys())
            model_keys = list(model.keys())
            for cmdId in model_keys:
                value = model.get(cmdId);
                print("ctrl cmdId:"+str(cmdId))
                print(value)
                if 2 == cmdId:                                      
                    Gpio_relay.write(1)
                    remaining_time = value
                    timer1.stop()
                    timer1.start(period=60 * 1000,mode=Timer.PERIODIC, callback=TimerCB)                    

        elif 5 == data[0] and 10211 == data[1]:
            res_data = dict()
            msg = data[2]
            pkgId = msg[0]
            
            for cmdId in msg[1]:
                if 1 == cmdId:
                    res_data[cmdId]=Gpio_relay.read()
                elif 2 == cmdId:
                    res_data[cmdId]=remaining_time      
            quecIot.phymodelAck(0, pkgId, res_data)
            print("read")
            print(res_data)
            
    def timerTask(self, t):
        global Gpio_relay
        global remaining_time
        remaining_time =remaining_time-1
        if 0 == remaining_time:        
            Gpio_relay.write(0)
            res_data = dict()
            res_data[1]=Gpio_relay.read()
            quecIot.phymodelReport(1,res_data)
            print("timeout")


if __name__ == '__main__':
    Quecthing()
