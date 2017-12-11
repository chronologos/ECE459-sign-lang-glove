import sys
import serial
import time
from subprocess import Popen, PIPE
import threading
from autocorrect import spell
import math

# Dynamic programming approach to min distance between strings
def editDistDP(str1, str2):
    m = len(str1)
    n = len(str2)
# Create a table to store results of subproblems
    dp = [[0 for x in range(n+1)] for x in range(m+1)]

# Fill d[][] in bottom up manner
    for i in range(m+1):
        for j in range(n+1):

        # If first string is empty, only option is to
        # isnert all characters of second string
            if i == 0:
                dp[i][j] = j # Min. operations = j

        # If second string is empty, only option is to
        # remove all characters of second string
            elif j == 0:
                dp[i][j] = i # Min. operations = i

        # If last characters are same, ignore last char
        # and recur for remaining string
            elif str1[i-1] == str2[j-1]:
                dp[i][j] = dp[i-1][j-1]

        # If last character are different, consider all
        # possibilities and find minimum
            else:
                dp[i][j] = 1 + min(dp[i][j-1],	 # Insert
                            dp[i-1][j],	 # Remove
                            dp[i-1][j-1]) # Replace
    return dp[m][n]

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
        self.charLen = 0
        self.totalWrong = 0
        self.totalLen = 0
        self.done = False
        self.username = username
        self.autocorrect_on = autocorrect
        # self.test = ["test\n" for i in range(0,10)]

    def runReader(self):
        input("READY: press <enter>!\n")
        while (1):
            # print("read")
            # c = self.test.pop()
            c = self.bt.read().decode("utf-8")

            self.item = c
            print(c)
            self.buffer += c

            if " " in self.buffer:
                ideal = input("Enter expected word: ")
                actual = self.buffer.split(" ")[0]
                if self.autocorrect_on:
                    actual_prime = spell(actual)
                    if actual_prime != actual:
                        print("Autocorrect corrected {0} to {1}".format(actual,actual_prime))
                        actual = actual_prime


                self.buffer = ""
                self.wrong=editDistDP(ideal, actual)
                self.totalWrong += self.wrong
                self.charLen = max(len(ideal), self.wrong)
                self.totalLen += self.charLen
                print(
                        "When signing {0}, {1} percent accuracy. \n Overall {2} percent accuracy for {3} chars signed\n".format(ideal,
                            (1 - abs(self.wrong/self.charLen))*100, (1 - abs(self.totalWrong/self.totalLen))*100, self.totalLen))
                self.done = True
                input("press <enter>!\n")
                self.bt.reset_input_buffer()
            # if self.done:
            #     with open("results.txt", "w+") as f:
            #         f.write(
            #             "{0}: {1} wrong {2} total {3} % correct".format(
            #                 self.username, self.wrong, self.total, float(self.total-self.wrong)/float(self.total)))
            #     return


def sendKey(key):
    if key == "!": # bs
        scpt = '''tell application \"System Events\" to key code 51'''
    elif key == "`": # space
        scpt = '''tell application \"System Events\" to keystroke space'''
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
    autocorrect = True if ("true" == sys.argv[2]) else False
    print("autocorrect_on = {0}".format(str(autocorrect)))
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
