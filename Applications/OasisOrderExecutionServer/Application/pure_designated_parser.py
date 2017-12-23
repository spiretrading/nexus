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
    if tokens[0] == 'Symbol':
      state = HEAD_SYMBOL
  else:
    if len(tokens[0]) == 0:
      pass
    if state == TAIL_SYMBOL:
      sys.stdout.write(',\n')
    else:
      state = TAIL_SYMBOL
    sys.stdout.write('"%s.TSX"' % tokens[0])

sys.stdout.write(']\n')
sys.stdout.write('...\n')
