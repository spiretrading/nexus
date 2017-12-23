import csv
import fileinput
import sys

reader = csv.reader(fileinput.input())
table = []
for tokens in reader:
  table.append((tokens[0], tokens[1], tokens[2]))

max_flag_length = 0
max_fee_length = 0
for entry in table:
  max_flag_length = max(max_flag_length, len(entry[0]))
  max_fee_length = max(max_fee_length, len(entry[2]))

for entry in table:
  flag = entry[0].ljust(max_flag_length)
  fee = entry[2].ljust(max_fee_length + 4)
  if len(entry[2]) > 0 and entry[2][0] != '-':
    fee = ' ' + fee[0:-1]
  comment = entry[1]
  sys.stdout.write('%s: %s# %s\n' % (flag, fee, comment))
