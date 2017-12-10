import sys
import serial
import time
from subprocess import Popen, PIPE
import threading
from autocorrect import spell

class InputMode:
    def __init__(self, port, autocorrect):
        self.item_available = False
        self.item = ""
        self.cv = threading.Condition()
        self.bt = serial.Serial(port, 9600)
        self.autocorrect_on = autocorrect
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
            if self.autocorrect_on:
                c_prime = spell(c)
                if c_prime != c:
                    print("autocorrect corrected {0} to {1}".format(c,c_prime))
                    c = c_prime
            self.item = c
            self.cv.acquire()
            self.item_available = True
            self.cv.notify()
            self.cv.release()


class TestMode:
    def __init__(self, port, username, autocorrect):
        self.item = ""
        self.buffer = ""
        self.bt = serial.Serial(port, 9600)
        self.wrong = 0
        self.total = 0
        self.done = False
        self.username = username
        self.autocorrect_on = autocorrect
        # self.test = ["test\n" for i in range(0,10)]

    def runReader(self):
        while (1):
            print("read")
            # c = self.test.pop()
            c = self.bt.read().decode("utf-8")
            if self.autocorrect_on:
                c_prime = spell(c)
                if c_prime != c:
                    print("autocorrect corrected {0} to {1}".format(c,c_prime))
                    c = c_prime
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
    if key == "!": # bs
        scpt = '''tell application \"System Events\" to keystroke space'''
    elif key == "`": # space
        scpt = '''tell application \"System Events\" to key code 51'''
    else:
        scpt = '''tell application \"System Events\" to keystroke \"''' + key + "\""
    scpt = scpt.encode('utf-8')
    args = []
    p = Popen(['osascript', '-'] + args, stdin=PIPE, stdout=PIPE, stderr=PIPE)
    stdout, stderr = p.communicate(scpt)


def run():
    if len(sys.argv) < 4:
        print("not enough arguments. Usage: python3 fake_kb.py serialport autocorrect_on mode (testopts)"
              + "\n modes: 1) INPUT 2) TEST"
                + "\n testopts: username")
        return
    port = sys.argv[1]
    autocorrect = if "true" == sys.argv[2] then True else False
    mode = sys.argv[3]
    if mode == "INPUT":
        t = InputMode(port, autocorrect)
        thread1 = threading.Thread(group=None, target=t.runReader, name="t1")
        thread2 = threading.Thread(group=None, target=t.runWriter, name="t2")
        thread1.start()
        thread2.start()
    if mode == "TEST":
        if len(sys.argv) != 5:
            print("not enough arguments. Usage: python3 fake_kb.py serialport autocorrect_on mode (testopts)"
                  + "\n modes: 1) INPUT 2) TEST"
                    + "\n testopts: username")
            return
        username = sys.argv[4]
        t = TestMode(port, username, autocorrect)
        thread1 = threading.Thread(group=None, target=t.runReader, name="t1")
        thread1.start()


if __name__ == "__main__":
    run()
