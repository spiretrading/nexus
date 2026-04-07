import csv
import sys


REQUIRED_HEADERS = ['official_name_en', 'ISO3166-1-Alpha-2',
  'ISO3166-1-Alpha-3', 'ISO3166-1-numeric']


def emit_yaml(countries):
  out = []
  out.append('---')
  out.append('countries:')
  for country in countries:
    name = "'" + country['name'].replace("'", "''") + "'"
    two_letter_code = "'" + country['two_letter_code'] + "'"
    three_letter_code = "'" + country['three_letter_code'] + "'"
    code = str(country['code'])
    out.append(f'  - name: {name}')
    out.append(f'    two_letter_code: {two_letter_code}')
    out.append(f'    three_letter_code: {three_letter_code}')
    out.append(f'    code: {code}')
  out.append('...')
  return '\n'.join(out) + '\n'


def main():
  reader = csv.DictReader(sys.stdin)
  headers = reader.fieldnames or []
  missing = [h for h in REQUIRED_HEADERS if h not in headers]
  if missing:
    sys.exit('Missing required header(s): ' + ', '.join(missing) +
      '\nHeaders seen: ' + ', '.join(headers))
  rows = []
  two_letter_codes = set()
  for row in reader:
    if row['is_independent'].strip() != 'Yes':
      continue
    name = row['official_name_en'].strip()
    two_letter_code = row['ISO3166-1-Alpha-2'].strip()
    three_letter_code = row['ISO3166-1-Alpha-3'].strip()
    code = row['ISO3166-1-numeric']
    code = int(code) if str(code).strip().isdigit() else None
    if not (
        name and two_letter_code and three_letter_code and code is not None):
      continue
    if two_letter_code in two_letter_codes:
      continue
    two_letter_codes.add(two_letter_code)
    rows.append({
      'name': name,
      'two_letter_code': two_letter_code,
      'three_letter_code': three_letter_code,
      'code': code,
    })
  yaml_text = emit_yaml(rows)
  sys.stdout.write(yaml_text)


if __name__ == '__main__':
  main()
