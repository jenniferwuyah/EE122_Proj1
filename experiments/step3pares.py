import os

"""
a = open("stat3a.txt", "r")
a_str = a.read()
a_split = a_str.split(".")
a_int = []
for item in a_split:
    a_int.append(float("." + item))
a_int = a_int[1::]

for num in a_int:
    print num
"""

a = open("stat3c.txt", "r")
a_str = a.read()
a_split = a_str.split(".")
a_int = []
for item in a_split:
    a_int.append(float("." + item))
a_int = a_int[1::]

for num in a_int:
    print num


