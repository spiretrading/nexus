import xlrd

def parse_tsx(out_file):
  book = xlrd.open_workbook('tsx_symbols.xls')
  sheet = book.sheets()[0]
  for row in xrange(10, sheet.nrows):
    out_file.write('  - symbol: ' +
      sheet.cell(row, 4).value.encode('ascii', 'ignore') + '.TSX\n')
    out_file.write('    name: ' +
      sheet.cell(row, 3).value.encode('ascii', 'ignore') + '\n')

def parse_nasdaq(out_file):
  in_file = open('nasdaqlisted.txt', 'r')
  c = 0
  for line in in_file:
    if c == 0:
      c += 1
      continue
    else:
      tokens = line.split('|')
      if tokens[0].find('File Creation Time') != -1:
        continue
      out_file.write('  - symbol: %s.NSDQ\n' % tokens[0].strip())
      description = tokens[1].strip()
      if description.find('-') != -1:
        description = description[0 : description.find('-')]
      description = description.strip()
      out_file.write('    name: %s\n' % description)

def parse_nyse(out_file):
  in_file = open('otherlisted.txt', 'r')
  c = 0
  for line in in_file:
    if c == 0:
      c += 1
      continue
    else:
      tokens = line.split('|')
      if tokens[0].find('File Creation Time') != -1:
        continue
      symbol = tokens[3].strip()
      symbol = symbol.replace('p', '.P').replace('w', '.V').replace('/', '.')
      exchange = tokens[2].strip()
      if exchange == 'A':
        exchange = 'AMEX'
      elif exchange == 'N':
        exchange = 'NYSE'
      elif exchange == 'P':
        exchange = 'ARCA'
      elif exchange == 'Z':
        exchange = 'BATS'
      out_file.write('  - symbol: %s.%s\n' % (symbol, exchange))
      description = tokens[1].strip()
      if description.find('-') != -1:
        description = description[0 : description.find('-')]
      description = description.strip()
      out_file.write('    name: %s\n' % description)

def main():
  out_file = open('symbols', 'w')
  out_file.write('---\n')
  out_file.write('symbols:\n')
  parse_tsx(out_file)
  parse_nasdaq(out_file)
  parse_nyse(out_file)
  out_file.write('...\n')

if __name__ == '__main__':
  main()
