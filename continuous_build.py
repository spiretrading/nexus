import argparse
import git
import os
import re
import shutil
import signal
import subprocess
import sys
import tarfile
import time
import zipfile


DRAIN_TIMEOUT = 30


def terminate(process):
  if sys.platform == 'win32':
    subprocess.call(['taskkill', '/F', '/T', '/PID', str(process.pid)],
      stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
  else:
    os.killpg(os.getpgid(process.pid), signal.SIGKILL)


def call(command, cwd, timeout):
  process = subprocess.Popen(
    command, stdout=subprocess.PIPE, cwd=cwd, stderr=subprocess.PIPE,
    start_new_session=True)
  try:
    output = process.communicate(timeout=timeout)
  except subprocess.TimeoutExpired:
    terminate(process)
    try:
      output = process.communicate(timeout=DRAIN_TIMEOUT)
    except subprocess.TimeoutExpired:
      output = (b'', b'')
    message = 'Timed out after %d seconds.' % timeout
    return (output[0], output[1] + message.encode('utf-8'), 1)
  return (output[0], output[1], process.returncode)


def make_tarfile(source, destination):
  with tarfile.open(destination, 'w:gz') as tar:
    for file in os.listdir(source):
      tar.add(os.path.join(source, file), arcname=file)


def make_zipfile(source, destination):
  source = os.path.abspath(source)
  with zipfile.ZipFile(destination, 'w', zipfile.ZIP_DEFLATED) as archive:
    for root, directories, files in os.walk(source):
      for entry in directories + files:
        entry_path = os.path.join(root, entry)
        archive.write(entry_path, arcname=os.path.relpath(entry_path, source))


def write_log(path, mode, sections):
  separator = os.linesep.encode('utf-8')
  with open(path, mode) as log_file:
    for title, text in sections:
      log_file.write(('=== %s ===' % title).encode('utf-8') + separator)
      log_file.write(text)
      log_file.write(separator + separator)


def executable_extension():
  if sys.platform == 'win32':
    return '.exe'
  else:
    return ''


def copy_build(applications, name, source, destination):
  binary = executable_extension()
  errors = []
  for application in applications:
    try:
      application_path = os.path.join(destination, application)
      os.makedirs(application_path, exist_ok=True)
      source_directory = os.path.join(source, 'Applications', application,
        'Application')
      executables = 0
      for file in os.listdir(source_directory):
        file_path = os.path.join(source_directory, file)
        if os.path.isdir(file_path):
          shutil.copytree(file_path, os.path.join(application_path, file),
            dirs_exist_ok=True)
          continue
        if not os.path.isfile(file_path):
          continue
        extension = os.path.splitext(file_path)[1]
        if extension == binary:
          executables += 1
        if extension in ['.py', '.yml', '.csv']:
          shutil.copy2(file_path, os.path.join(application_path, file))
        if sys.platform == 'win32':
          if extension in ['.bat', '.exe']:
            shutil.copy2(file_path, os.path.join(application_path, file))
        else:
          if extension in ['', '.sh']:
            shutil.copy2(file_path, os.path.join(application_path, file))
      if executables == 0:
        errors.append('%s %s produced no executable.' % (name, application))
    except OSError as e:
      errors.append('Failed to copy %s %s: %s' % (name, application, e))
  return errors


def clean_build(applications, source):
  binary = executable_extension()
  for application in applications:
    source_directory = os.path.join(source, 'Applications', application,
      'Application')
    if not os.path.isdir(source_directory):
      continue
    for file in os.listdir(source_directory):
      file_path = os.path.join(source_directory, file)
      if not os.path.isfile(file_path):
        continue
      if os.path.splitext(file_path)[1] == binary:
        os.remove(file_path)


def python_extension():
  if sys.platform == 'win32':
    return '.pyd'
  else:
    return '.so'


def nexus_library(repo_path):
  return os.path.join(repo_path, 'Nexus', 'Libraries', 'Release',
    'nexus%s' % python_extension())


def python_libraries(repo_path):
  return [os.path.join(repo_path, 'Nexus', 'Dependencies', 'aspen',
    'Libraries', 'Release', 'aspen%s' % python_extension()),
    os.path.join(repo_path, 'Nexus', 'Dependencies', 'Beam', 'Beam',
    'Libraries', 'Release', 'beam%s' % python_extension()),
    nexus_library(repo_path)]


def copy_python_libraries(destination, repo_path):
  python_path = os.path.join(destination, 'Python')
  os.makedirs(python_path, exist_ok=True)
  errors = []
  for library in python_libraries(repo_path):
    if os.path.isfile(library):
      shutil.copy2(library, python_path)
    else:
      errors.append('%s was not built.' % os.path.basename(library))
  return errors


def clean_nexus_library(repo_path):
  library = nexus_library(repo_path)
  if os.path.isfile(library):
    os.remove(library)


def build_repo(repo, path, timeout):
  commits = repo.git.rev_list('--first-parent', 'HEAD').split('\n')
  commits.reverse()
  builds = [int(d) for d in os.listdir(path)
    if d.isdecimal() and os.path.isdir(os.path.join(path, d))]
  builds.sort(reverse=True)
  latest = None
  for version in builds:
    if 0 < version <= len(commits):
      latest = version
      break
  if latest is None:
    versions = [len(commits)]
  else:
    versions = range(latest + 1, len(commits) + 1)
  if sys.platform == 'win32':
    extension = 'bat'
  else:
    extension = 'sh'
  nexus_applications = ['AdministrationServer', 'ChartingServer',
    'ComplianceServer', 'DefinitionsServer', 'MarketDataRelayServer',
    'MarketDataServer', 'ReplayMarketDataFeedClient', 'RiskServer',
    'SimulationMarketDataFeedClient', 'SimulationOrderExecutionServer',
    'WebPortal']
  if sys.platform == 'win32':
    nexus_applications.extend(['Lollipop', 'Spire'])
  beam_applications = ['AdminClient', 'ServiceLocator', 'UidServer']
  beam_path = os.path.join(repo.working_dir, 'Nexus', 'Dependencies', 'Beam')
  staging_path = os.path.join(path, 'staging')
  for version in versions:
    commit = commits[version - 1]
    repo.git.checkout(commit)
    clean_build(nexus_applications, repo.working_dir)
    clean_nexus_library(repo.working_dir)
    result = []
    for step in ['configure', 'build']:
      output = call(
        [os.path.join(repo.working_dir, '%s.%s' % (step, extension))],
        repo.working_dir, timeout)
      result.append((step, output))
      status = output[2]
      if status != 0:
        break
    sections = []
    for step, output in result:
      sections.append((step, output[0]))
      sections.append(('%s errors' % step, output[1]))
    if os.path.isdir(staging_path):
      shutil.rmtree(staging_path)
    package_path = os.path.join(staging_path, 'package')
    os.makedirs(package_path)
    log_path = os.path.join(staging_path, 'build.txt')
    write_log(log_path, 'wb', sections)
    archive_path = None
    try:
      if status == 0:
        errors = copy_build(nexus_applications, 'Nexus', repo.working_dir,
          package_path)
        errors.extend(
          copy_build(beam_applications, 'Beam', beam_path, package_path))
        shutil.copy2(os.path.join(repo.working_dir, 'Applications',
          'setup.py'), os.path.join(package_path, 'setup.py'))
        shutil.copytree(os.path.join(repo.working_dir, 'Applications',
          'Python'), os.path.join(package_path, 'Python'), dirs_exist_ok=True)
        errors.extend(copy_python_libraries(package_path, repo.working_dir))
        if len(errors) != 0:
          write_log(log_path, 'ab',
            [('copy errors', os.linesep.join(errors).encode('utf-8'))])
        else:
          if sys.platform == 'win32':
            file = 'install_python.bat'
            shutil.copy2(os.path.join(repo.working_dir, 'Applications', file),
              os.path.join(package_path, file))
            archive_path = os.path.join(staging_path,
              'nexus-%s.zip' % version)
            make_zipfile(package_path, archive_path)
          else:
            for file in ['check.sh', 'install_python.sh', 'start.sh',
                'stop.sh']:
              copy_path = os.path.join(package_path, file)
              shutil.copy2(os.path.join(repo.working_dir, 'Applications',
                file), copy_path)
              with open(copy_path, 'r') as f:
                translation = re.sub(r'/Application\b', '', f.read())
              with open(copy_path, 'w') as f:
                f.write(translation)
            archive_path = os.path.join(staging_path,
              'nexus-%s.tar.gz' % version)
            make_tarfile(package_path, archive_path)
    except Exception as e:
      archive_path = None
      write_log(log_path, 'ab', [('packaging error', str(e).encode('utf-8'))])
    destination_path = os.path.join(path, str(version))
    os.makedirs(destination_path, exist_ok=True)
    shutil.move(log_path, os.path.join(destination_path, 'build.txt'))
    if archive_path is not None:
      shutil.move(archive_path, destination_path)
    shutil.rmtree(staging_path)
  return len(versions)


def main():
  parser = argparse.ArgumentParser(
    description='v1.0 Copyright (C) 2020 Spire Trading Inc.')
  parser.add_argument('-p', '--path', type=str, help='Destination path.',
    required=True)
  parser.add_argument('-t', '--period', type=int, help='Time period.',
    default=600)
  parser.add_argument('--timeout', type=int,
    help='Build timeout in seconds.', default=14400)
  args = parser.parse_args()
  os.makedirs(args.path, exist_ok=True)
  repo = git.Repo('.')
  try:
    branch = repo.active_branch.name
  except TypeError:
    sys.exit('HEAD is detached, check out the branch to build first.')
  while True:
    try:
      repo.git.fetch()
      repo.git.checkout('--force', branch)
      repo.git.reset('--hard', '@{upstream}')
    except Exception as e:
      print('Failed to pull: %s' % e)
    built = 0
    try:
      built = build_repo(repo, args.path, args.timeout)
    except Exception as e:
      print('Failed to build: %s' % e)
    try:
      repo.git.checkout(branch)
    except Exception as e:
      print('Failed to check out %s: %s' % (branch, e))
    if built == 0:
      time.sleep(args.period)


if __name__ == '__main__':
  main()
