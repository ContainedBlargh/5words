from PyDictionary import PyDictionary
from sys import stdin, stdout

dc = PyDictionary()

for line in stdin:
    if (dc.meaning(line.strip(), True) is not None):
        stdout.write(line.strip())
        stdout.write("\n")
        stdout.flush()
