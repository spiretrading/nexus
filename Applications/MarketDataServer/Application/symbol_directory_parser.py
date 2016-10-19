import xlrd

def parse_tsx(out_file):
  book = xlrd.open_workbook('tsx_symbols.xls')
  sheet = book.sheets()[0]
  for row in xrange(10, sheet.nrows):
    symbol = '%s.TSX' % sheet.cell(row, 4).value.encode('ascii', 'ignore')
    name = sheet.cell(row, 3).value.encode('ascii', 'ignore')
    out_file.write('  - symbol: %s\n' % symbol)
    out_file.write('    name: "%s"\n' % name)

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
      symbol = '%s.NSDQ' % tokens[0].strip()
      description = tokens[1].strip()
      if description.find('-') != -1:
        description = description[0 : description.rfind('-')]
      description = description.strip()
      out_file.write('  - symbol: "%s"\n' % symbol)
      out_file.write('    name: "%s"\n' % description)

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
      entry = tokens[3].strip()
      symbol = ''
      suffix = ''
      token = 0
      size = len(entry)
      state = 0
      while size > 0:
        if state == 0:
          if entry[token] == '/' or entry[token] == 'p' or entry[token] == 'r' \
              or entry[token] == 'w':
            state = 1
          elif entry[token] != ' ':
            symbol += entry[token]
            token += 1
            size -= 1
          else:
            token += size
            size = 0
        if state == 1:
          if entry[token] != ' ':
            suffix += entry[token]
            token += 1
            size -= 1
          else:
            token += size
            size = 0
      if len(suffix) != 0:
        symbol += '.'
        token = 0
        size = len(suffix)
        while size != 0:
          if suffix[token] == 'p':
            symbol += 'PR'
            token += 1
            size -= 1
          elif suffix[token] == 'r':
            symbol += 'RT'
            token += 1
            size -= 1
          elif suffix[token] == 'w':
            symbol += 'WI'
            token += 1
            size -= 1
          elif suffix[token] == '/':
            token += 1
            size -= 1
          else:
            symbol += suffix[token]
            token += 1
            size -= 1
      exchange = tokens[2].strip()
      if exchange == 'A':
        exchange = 'AMEX'
      elif exchange == 'N':
        exchange = 'NYSE'
      elif exchange == 'P':
        exchange = 'ARCA'
      elif exchange == 'Z':
        exchange = 'BATS'
      out_file.write('  - symbol: "%s.%s"\n' % (symbol, exchange))
      description = tokens[1].strip()
      if description.find('-') != -1:
        description = description[0 : description.find('-')]
      description = description.strip()
      out_file.write('    name: "%s"\n' % description)

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
