import argparse
import os
import sys

def output(tokens, destination, address, interface, password):
  host = tokens[3]
  port = tokens[4]
  destination.write('---\n')
  destination.write('service_locator:\n')
  destination.write('  address: %s:20000\n' % address)
  destination.write('  username: market_data_feed\n')
  destination.write('  password: "%s"\n' % password)
  destination.write('\n')
  destination.write('enable_logging: false\n')
  destination.write('market: XNYS\n')
  destination.write('sampling: 100ms\n')
  destination.write('host: %s:%s\n' % (host, port))
  destination.write('interface: %s:%s\n' % (interface, port))
  destination.write('market_codes:\n')
  destination.write('  - code: A\n')
  destination.write('    market: XASE\n')
  destination.write('  - code: B\n')
  destination.write('    market: XBOS\n')
  destination.write('  - code: C\n')
  destination.write('    market: XCIS\n')
  destination.write('  - code: D\n')
  destination.write('    market: XADF\n')
  destination.write('  - code: I\n')
  destination.write('    market: XISX\n')
  destination.write('  - code: J\n')
  destination.write('    market: EDGA\n')
  destination.write('  - code: K\n')
  destination.write('    market: EDGX\n')
  destination.write('  - code: M\n')
  destination.write('    market: XCHI\n')
  destination.write('  - code: N\n')
  destination.write('    market: XNYS\n')
  destination.write('  - code: P\n')
  destination.write('    market: ARCX\n')
  destination.write('  - code: T\n')
  destination.write('    market: XNAS\n')
  destination.write('  - code: V\n')
  destination.write('    market: IEXG\n')
  destination.write('  - code: W\n')
  destination.write('    market: XCBO\n')
  destination.write('  - code: X\n')
  destination.write('    market: XPHL\n')
  destination.write('  - code: Y\n')
  destination.write('    market: BATY\n')
  destination.write('  - code: Z\n')
  destination.write('    market: BATS\n')
  destination.write('...\n')

def main():
  parser = argparse.ArgumentParser(
    description='v1.0 Copyright (C) 2009 Eidolon Systems Ltd.')
  parser.add_argument('-a', '--address', type=str, help='Address',
    required=True)
  parser.add_argument('-l', '--local', type=str, help='Local Interface',
    required=True)
  parser.add_argument('-p', '--password', type=str, help='Password',
    required=True)
  args = parser.parse_args()
  for line in sys.stdin:
    tokens = line.strip().split(' ')
    partition = tokens[1].lower()[0 : tokens[1].find('/')]
    directory = 'cta_%s%s%s' % (tokens[0].lower(), tokens[2].lower(), partition)
    if not os.path.exists(directory):
      os.mkdir(directory)
    destination = open('%s/config.yml' % directory, 'w')
    output(tokens, destination, args.address, args.local, args.password)

if __name__ == '__main__':
  main()
