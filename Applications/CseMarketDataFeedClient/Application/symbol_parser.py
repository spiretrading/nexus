import argparse
import csv
import yaml

def main():
  parser = argparse.ArgumentParser(
    description='v1.0 Copyright (C) 2026 Spire Trading Inc.')
  parser.add_argument('-f', '--file', type=str, help='Symbol file.',
    required=True)
  parser.add_argument('-o', '--out', type=str, help='Output file.',
    default='symbols.yml')
  args = parser.parse_args()
  symbols = []
  with open(args.file, 'r', encoding='utf-8') as csv_file:
    reader = csv.DictReader(csv_file)
    for row in reader:
      if row['Currency'] == 'CAD' and row['Listing Market'] == 'CSE':
        symbols.append({
          'symbol': row['Symbol'],
          'name': row['Security Name'],
          'board_lot': int(row['Lot Size'])
        })
  with open(args.out, 'w', encoding='utf-8') as yaml_file:
    yaml.dump(symbols, yaml_file, default_flow_style=False, sort_keys=False,
      explicit_start=True, explicit_end=True)

if __name__ == '__main__':
  main()
