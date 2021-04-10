#!/usr/bin/env python3

import sys

index = int(sys.argv[1])
length = int("1" if len(sys.argv) < 3 else sys.argv[2])

content = open("example/ex.lll").read()

def getLocation(index):
  line_number = 1;
  char_number = 1;
  for c in enumerate(content):
    if c[0] == index:
      return (line_number, char_number)

    char_number += 1
    if c[1] == '\n':
      line_number += 1
      char_number = 1

  raise

def getLine(index):
  count = 1;
  buffer = "";
  for c in content:
    if c == '\n':
      if count == index:
        return buffer

      buffer = ""
      count += 1
    else:
      buffer += c
  return buffer

location = getLocation(index)
line = getLine(location[0])
trimmed = line.strip().replace('\n', '')
print('{:>5s} {}'.format(str(location[0]), trimmed))

print(' ' * (6 + location[1] - 1 - (len(line) - len(trimmed))), end='')
print('^' * length)
