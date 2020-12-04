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

if __name__ == '__main__':
  main()
