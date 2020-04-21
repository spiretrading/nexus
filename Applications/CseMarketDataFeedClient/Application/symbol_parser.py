import argparse
import csv

def main():
  parser = argparse.ArgumentParser(
    description='v1.0 Copyright (C) 2020 Spire Trading Inc.')
  parser.add_argument('-f', '--file', type=str, help='Symbol file.',
    required=True)
  parser.add_argument('-o', '--out', type=str, help='Output file.',
    default='symbols.yml')
  args = parser.parse_args()
  with open(args.file, 'r') as input, open(args.out, 'w') as output:
    output.write('---\n')
    symbol_reader = csv.reader(input)
    next(symbol_reader, None)
    for row in symbol_reader:
      if row[4] == 'CAD' and row[5] == 'CSE':
        output.write('  - symbol: %s\n' % row[1])
        output.write('    name: %s\n' % row[2])
        output.write('    board_lot: %s\n' % row[6])
    output.write('...\n')

if __name__ == '__main__':
  main()
