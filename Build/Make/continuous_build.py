import argparse
import distutils
from distutils import dir_util
import ftplib
import git
import os
import shutil
import subprocess
import time

def ftp_upload(ftp, path):
  for name in os.listdir(path):
    local_path = os.path.join(path, name)
    if os.path.isfile(local_path):
      ftp.storbinary('STOR %s' % name, open(local_path, 'rb'))
    elif os.path.isdir(local_path):
      try:
        ftp.mkd(name)
      except ftplib.error_perm as e:
        if not e.args[0].startswith('550'):
          raise
      ftp.cwd(name)
      ftp_upload(ftp, local_path)
      ftp.cwd('..')

def build_repo(repo, name, ftp_site, username, password):
  try:
    shutil.rmtree('./%s' % name)
  except OSError:
    pass
  repo = git.Repo.clone_from(repo, './%s' % name)
  commits = sorted([commit for commit in repo.iter_commits('master')],
    key = lambda commit: -int(commit.committed_date))
  ftp = ftplib.FTP(ftp_site)
  ftp.login(username, password)
  ftp.cwd('Make/%s' % name)
  builds = [int(n) for n in ftp.nlst()]
  ftp.close()
  builds.sort(reverse=True)
  if len(builds) == 0:
    builds.append(int(commits[1].committed_date))
  for i in range(len(commits)):
    commit = commits[i]
    timestamp = int(commit.committed_date)
    if timestamp in builds:
      commits = commits[0:i]
      commits.reverse()
      break
  for commit in commits:
    timestamp = int(commit.committed_date)
    repo.git.checkout(commit.hexsha)
    terminal_output = subprocess.Popen(['sh', '-c', './%s/Build/Make/setup.sh' %
      name], stdout=subprocess.PIPE).communicate()[0]
    os.chdir('./%s/Build/Make/' % name)
    terminal_output += '\n\n\n\n'
    terminal_output += subprocess.Popen(['sh', '-c', './run_cmake.sh'],
      stdout=subprocess.PIPE).communicate()[0]
    terminal_output += subprocess.Popen(['sh', '-c', './build.sh'],
      stdout=subprocess.PIPE).communicate()[0]
    os.chdir('./../../../')
    os.makedirs(str(timestamp))
    log_file = open('./%s/build.log' % str(timestamp), 'w')
    log_file.write(terminal_output)
    log_file.close()
    applications = ['AdministrationServer', 'AsxItchMarketDataFeedClient',
      'ChartingServer', 'ChiaMarketDataFeedClient', 'ClientWebPortal',
      'ComplianceServer', 'CtaMarketDataFeedClient', 'DefinitionsServer',
      'MarketDataClientStressTest', 'MarketDataClientTemplate',
      'MarketDataRelayServer', 'MarketDataServer', 'OrderExecutionBackup',
      'OrderExecutionRestore', 'RiskServer', 'SimulationMarketDataFeedClient',
      'SimulationOrderExecutionServer', 'TmxIpMarketDataFeedClient',
      'TmxTl1MarketDataFeedClient', 'UtpMarketDataFeedClient']
    for application in applications:
      dir_util.copy_tree('./%s/Applications/%s/Application' %
        (name, application), './%s/Applications/%s' %
        (str(timestamp), application))
    dir_util.copy_tree('./%s/%s/Library/Release' % (name, name),
      './%s/Libraries' % str(timestamp))
    ftp = ftplib.FTP(ftp_site)
    ftp.login(username, password)
    ftp.cwd('Make/%s' % name)
    path = './%s' % str(timestamp)
    ftp.mkd(os.path.basename(path))
    ftp.cwd(os.path.basename(path))
    ftp_upload(ftp, path)
    shutil.rmtree(path)

def main():
  parser = argparse.ArgumentParser(
    description='v1.0 Copyright (C) 2017 Eidolon Systems Ltd.')
  parser.add_argument('-r', '--repo', type=str, help='Remote repository.',
    required=True)
  parser.add_argument('-n', '--name', type=str, help='Repository name.',
    required=True)
  parser.add_argument('-f', '--ftp', type=str, help='FTP address',
    required=True)
  parser.add_argument('-u', '--username', type=str, help='Username',
    default='anonymous')
  parser.add_argument('-p', '--password', type=str, help='Password',
    default='')
  parser.add_argument('-t', '--period', type=int, help='Time period.',
    default=600)
  args = parser.parse_args()
  while True:
    build_repo(args.repo, args.name, args.ftp, args.username, args.password)
    time.sleep(args.period)

if __name__ == '__main__':
  main()
