import sys
import serial
import time
from subprocess import Popen, PIPE
import threading
from autocorrect import spell

class InputMode:
    def __init__(self, port):
        self.item_available = False
        self.item = ""
        self.cv = threading.Condition()
        self.bt = serial.Serial(port, 9600)

    def runWriter(self):
        while (1):
            self.cv.acquire()
            while not self.item_available:
                self.cv.wait()
            self.item_available = False
            _ = [sendKey(x) for x in self.item]
            self.cv.release()

    def runReader(self):
        while (1):
            # c = input("")
            c = self.bt.read().decode("utf-8")
            self.item = c
            self.cv.acquire()
            self.item_available = True
            self.cv.notify()
            self.cv.release()


class TestMode:
    def __init__(self, port, username):
        self.item = ""
        self.buffer = ""
        self.bt = serial.Serial(port, 9600)
        self.wrong = 0
        self.total = 0
        self.done = False
        self.username = username
        # self.test = ["test\n" for i in range(0,10)]

    def runReader(self):
        while (1):
            print("read")
            # c = self.test.pop()
            c = self.bt.read().decode("utf-8")
            self.item = c
            print(c)
            self.buffer += c

            if "\n" in self.buffer:
                actual = self.buffer.split("\n")[0]
                ideal = input("expected word was: ")
                self.buffer = ""
                if actual != ideal:
                    self.wrong += 1
                self.total += 1
                if self.total == 10:
                    print(
                        "job's done, total = {0}, wrong = {1}".format(
                            self.total, self.wrong))
                    self.done = True
            if self.done:
                with open("results.txt", "w+") as f:
                    f.write(
                        "{0}: {1} wrong {2} total {3} % correct".format(
                            self.username, self.wrong, self.total, float(self.total-self.wrong)/float(self.total)))
                return


def sendKey(key):
    scpt = '''tell application \"System Events\" to keystroke \"''' + key + "\""
    scpt = scpt.encode('utf-8')
    args = []
    p = Popen(['osascript', '-'] + args, stdin=PIPE, stdout=PIPE, stderr=PIPE)
    stdout, stderr = p.communicate(scpt)


def run():
    if len(sys.argv) < 3:
        print("not enough arguments. Usage: python3 fake_kb.py serialport mode (testopts)"
              + "\n modes: 1) INPUT 2) TEST"
                + "\n testopts: username")
        return
    port = sys.argv[1]
    mode = sys.argv[2]
    if mode == "INPUT":
        t = InputMode(port)
        thread1 = threading.Thread(group=None, target=t.runReader, name="t1")
        thread2 = threading.Thread(group=None, target=t.runWriter, name="t2")
        thread1.start()
        thread2.start()
    if mode == "TEST":
        if len(sys.argv) != 4:
            print("not enough arguments. Usage: python3 fake_kb.py serialport mode (testopts)"
                  + "\n modes: 1) INPUT 2) TEST"
                    + "\n testopts: username")
            return
        username = sys.argv[3]
        t = TestMode(port, username)
        thread1 = threading.Thread(group=None, target=t.runReader, name="t1")
        thread1.start()


if __name__ == "__main__":
    run()
