import argparse
import copy
import os

DATE_LENGTH = 21
REVOKE = 0
GRANT = 1

class MonthlyReport:
  def __init__(self, date):
    self.date = date
    self.added_entitlements = {}
    self.removed_entitlements = {}

  def incorporate(self, previous_report):
    self.added_entitlements = copy.deepcopy(previous_report.added_entitlements)
    for entitlement in previous_report.removed_entitlements:
      for account in previous_report.removed_entitlements[entitlement]:
        if account in self.added_entitlements[entitlement]:
          self.added_entitlements[entitlement].remove(account)

  def print_report(self, list_accounts):
    print(self.date)
    for entitlement in self.added_entitlements:
      accounts = self.added_entitlements[entitlement]
      print('\t%s %s' % (entitlement, len(accounts)))
      if list_accounts:
        for account in accounts:
          print('\t\t%s' % account)

  def grant(self, account, entitlement):
    if entitlement not in self.added_entitlements:
      self.added_entitlements[entitlement] = []
    if entitlement not in self.removed_entitlements:
      self.removed_entitlements[entitlement] = []
    if account not in self.added_entitlements[entitlement]:
      self.added_entitlements[entitlement].append(account)
    if account in self.removed_entitlements[entitlement]:
      self.removed_entitlements[entitlement].remove(account)

  def revoke(self, account, entitlement):
    if entitlement not in self.removed_entitlements:
      self.removed_entitlements[entitlement] = []
    self.removed_entitlements[entitlement].append(account)

def main():
  parser = argparse.ArgumentParser(
    description='v1.0 Copyright (C) 2020 Spire Trading Inc.')
  parser.add_argument('-d', '--directory', type=str, help='Log directory',
    default='.')
  parser.add_argument('-a', '--accounts', help='List all accounts',
    action = 'store_true')
  args = parser.parse_args()
  current_report = None
  for filename in os.listdir(args.directory):
    if os.path.splitext(filename)[1] != '.log':
      continue
    for line in open(os.path.join(args.directory, filename), 'r'):
      line = line.strip()
      if line.find('entitlement') == -1:
        continue
      date = line[0 : DATE_LENGTH - 1][0: 8]
      if current_report is None:
        current_report = MonthlyReport(date)
      elif current_report.date != date:
        next_report = MonthlyReport(date)
        next_report.incorporate(current_report)
        current_report.print_report(args.accounts)
        current_report = next_report
      remainder = line[DATE_LENGTH + 1:]
      first_quote = remainder.find('"') + 1
      last_quote = remainder.find('"', first_quote)
      entitlement = remainder[first_quote : last_quote]
      account_index = remainder.rfind(' ')
      account = remainder[account_index + 1 : -1]
      if account.find('service') != -1:
        continue
      if remainder.find('grants') != -1:
        current_report.grant(account, entitlement)
      elif remainder.find('revokes') != -1:
        current_report.revoke(account, entitlement)
  current_report.print_report(args.accounts)

if __name__ == '__main__':
  main()
