import argparse
import distutils
from distutils import dir_util
import git
import os
import shutil
import subprocess
import time

def call(command):
  return subprocess.Popen(command, shell=True, executable='/bin/bash',
    stdout=subprocess.PIPE, stderr=subprocess.PIPE).communicate()

def user_call(command):
  return call('sudo -u $(logname) %s' % command)

def copy_build(applications, timestamp, name):
  for application in applications:
    user_call('mkdir -p ./%s/Applications/%s' % (str(timestamp), application))
    user_call('cp -r ./%s/Applications/%s/Application ./%s/Applications/%s' %
      (name, application, str(timestamp), application))
  user_call('mkdir -p ./%s/Libraries' % str(timestamp))
  user_call('cp -r ./%s/%s/Library/Release ./%s/Libraries' %
    (name, name, str(timestamp)))

def build_repo(repo, path):
  try:
    shutil.rmtree('./Nexus')
  except OSError:
    pass
  user_call('git clone %s Nexus' % repo)
  repo = git.Repo('./Nexus')
  commits = sorted([commit for commit in repo.iter_commits('master')],
    key = lambda commit: -int(commit.committed_date))
  builds = [int(d) for d in os.listdir(path) if os.path.isdir(
    os.path.join(path, d))]
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
    user_call('pushd Nexus')
    user_call('git checkout %s' % commit.hexsha)
    user_call('popd')
    result = []
    result.append(call('./Nexus/Build/Make/setup.sh'))
    os.chdir('./Nexus/Build/Make/')
    result.append(user_call('./run_cmake.sh'))
    result.append(user_call('./build.sh'))
    terminal_output = ''
    for output in result:
      terminal_output += output[0] + '\n\n\n\n'
    for output in result:
      terminal_output += output[1] + '\n\n\n\n'
    os.chdir('./../../../')
    user_call('mkdir %s' % str(timestamp))
    nexus_applications = ['AdministrationServer', 'AsxItchMarketDataFeedClient',
      'ChartingServer', 'ChiaMarketDataFeedClient', 'ClientWebPortal',
      'ComplianceServer', 'CseMarketDataFeedClient', 'CtaMarketDataFeedClient',
      'DefinitionsServer', 'MarketDataClientStressTest',
      'MarketDataClientTemplate', 'MarketDataRelayServer', 'MarketDataServer',
      'OrderExecutionBackup', 'OrderExecutionRestore', 'RiskServer',
      'SimulationMarketDataFeedClient', 'SimulationOrderExecutionServer',
      'TmxIpMarketDataFeedClient', 'TmxTl1MarketDataFeedClient',
      'UtpMarketDataFeedClient']
    copy_build(nexus_applications, timestamp, 'Nexus')
    beam_applications = ['AdminClient', 'RegistryServer', 'ServiceLocator',
      'UidServer']
    copy_build(beam_applications, timestamp, 'Beam')
    user_call('cp ./Nexus/Applications/*.sh ./%s/Applications' %
      str(timestamp))
    user_call('cp ./Nexus/Applications/*.sql ./%s/Applications' %
      str(timestamp))
    log_file = open('./%s/build.log' % str(timestamp), 'w')
    log_file.write(terminal_output)
    log_file.close()
    user_call('chown $(logname) ./%s/build.log' % str(timestamp))
    user_call('chgrp $(logname) ./%s/build.log' % str(timestamp))
    destination = os.path.join(path, str(timestamp))
    user_call('mv %s %s' % (str(timestamp), destination))

def main():
  parser = argparse.ArgumentParser(
    description='v1.0 Copyright (C) 2017 Eidolon Systems Ltd.')
  parser.add_argument('-r', '--repo', type=str, help='Remote repository.',
    required=True)
  parser.add_argument('-p', '--path', type=str, help='Destination path.',
    required=True)
  parser.add_argument('-t', '--period', type=int, help='Time period.',
    default=600)
  args = parser.parse_args()
  while True:
    build_repo(args.repo, args.path)
    time.sleep(args.period)

if __name__ == '__main__':
  main()
