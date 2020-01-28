import sys

import urllib.request
import xlrd

URL = 'https://www.hkex.com.hk/eng/services/trading/securities/securitieslists/ListOfSecurities.xlsx'

def main():
  with urllib.request.urlopen(URL) as response, open('hkex.xlsx', 'wb') as \
      destination:
    destination.write(response.read())
  book = xlrd.open_workbook('hkex.xlsx')
  sheet = book.sheet_by_index(0)
  STOCK_COLUMN = None
  STAMP_COLUMN = None
  for c in range(0, sheet.ncols):
    if sheet.row_values(2)[c] == 'Stock Code':
      STOCK_COLUMN = c
    elif sheet.row_values(2)[c] == 'Subject to Stamp Duty':
      STAMP_COLUMN = c
  if STOCK_COLUMN is None:
    print('Stock code column not found.')
    sys.exit(-1)
  elif STAMP_COLUMN is None:
    print('Stamp column not found.')
    sys.exit(-1)
  sys.stdout.write('---\n')
  sys.stdout.write('symbols: [\n')
  for i in range(3, sheet.nrows):
    if sheet.row_values(i)[STAMP_COLUMN] == 'Y':
      if i != 3:
        sys.stdout.write(',\n')
      code = int(sheet.row_values(i)[STOCK_COLUMN])
      sys.stdout.write('"%s.HKEX"' % code)
  sys.stdout.write(']\n')
  sys.stdout.write('...\n')

if __name__ == '__main__':
  main()
