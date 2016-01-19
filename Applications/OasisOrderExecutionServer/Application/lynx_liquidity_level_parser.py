import fileinput
import sys

HEADER = 0
SYMBOL = 1

state = HEADER

sys.stdout.write('---\n')
sys.stdout.write('symbols:\n')

for line in fileinput.input():
  tokens = line.strip().split(',')
  if state == HEADER:
    if tokens[0] == 'Symbol':
      state = SYMBOL
  else:
    if len(tokens[0]) == 0:
      pass
    sys.stdout.write('  - symbol: "%s.%s"\n' % (tokens[0], tokens[1]))
    sys.stdout.write('    level: "%s"\n' % tokens[3])

sys.stdout.write('...\n')
