import sys
import serial
import time
from subprocess import Popen, PIPE
import threading
# from autocorrect import spell


class T1:
    def __init__(self):
        self.item_available = False
        self.item = ""
        self.cv = threading.Condition()
        print("thread started")
    def runReader(self):
        while(1):
            self.cv.acquire()
            while not self.item_available:
                self.cv.wait()
            self.item_available = False
            print("reader read {0}".format(self.item))
            self.cv.release()
    def runWriter(self):
        while (1):
            c = input("")
            self.cv.acquire()
            self.item = c
            print("writer got {0}".format(self.item))
            self.item_available = True
            self.cv.notify()
            self.cv.release()

def sendKey(key):
    scpt = '''tell application \"System Events\" to keystroke \"''' + key + "\""
    scpt = scpt.encode('utf-8')
    args=[]
    p = Popen(['osascript', '-'] + args, stdin=PIPE, stdout=PIPE, stderr=PIPE)
    stdout, stderr = p.communicate(scpt)

if __name__ == "__main__":
    print("main thread started")
    t = T1()
    thread1 = threading.Thread(group=None,target=t.runReader,name="t1")
    thread2 = threading.Thread(group=None,target=t.runWriter,name="t1")
    thread1.start()
    thread2.start()

# if __name__ == "__main__":

    # serialport = serial.Serial(sys.argv[1], 9600, timeout=5)
    # while True:
        # command = serialport.read().decode("utf-8") 
        # sendKey(command)
        # # sendKey(sys.argv[1])
    # serialport.close()
