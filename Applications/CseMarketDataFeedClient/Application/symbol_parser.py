import xlrd

book = xlrd.open_workbook('cse.xls')
first_sheet = book.sheet_by_index(0)
print '---'
for i in range(5, first_sheet.nrows):
  symbol = first_sheet.cell(i, 1).value.split('\n')[0]
  name = first_sheet.cell(i, 0).value.split('\n')[0]
  if len(symbol) == 0 or len(name) == 0:
    break
  print '  - symbol: %s' % symbol
  print '    name: %s' % name
print '...'
