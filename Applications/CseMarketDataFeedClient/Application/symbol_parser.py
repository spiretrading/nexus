import string
import xlrd

printable = set(string.printable)
output = open('symbols.yml', 'w')
book = xlrd.open_workbook('cse.xls')
first_sheet = book.sheet_by_index(0)
output.write('---\n')
for i in range(5, first_sheet.nrows):
  symbol = filter(lambda x: x in printable,
    first_sheet.cell(i, 1).value.split('\n')[0])
  name = filter(lambda x: x in printable,
    first_sheet.cell(i, 0).value.split('\n')[0])
  if len(symbol) == 0 or len(name) == 0:
    break
  output.write('  - symbol: %s\n' % symbol)
  output.write('    name: %s\n' % name)
output.write('...\n')
