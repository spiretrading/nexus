import fileinput
import sys

HEADER = 0
HEAD_SYMBOL = 1
TAIL_SYMBOL = 2

state = HEADER

sys.stdout.write('---\n')
sys.stdout.write('symbols: [\n')

for line in fileinput.input():
  tokens = line.strip().split(',')
  if state == HEADER:
    if tokens[0] == 'Name' and tokens[1] == 'Ticker':
      state = HEAD_SYMBOL
  else:
    if len(tokens[1]) == 0:
      pass
    if state == TAIL_SYMBOL:
      sys.stdout.write(',\n')
    else:
      state = TAIL_SYMBOL
    sys.stdout.write('"%s.TSX"' % tokens[1])

sys.stdout.write(']\n')
sys.stdout.write('...\n')
