const { spawnSync } = require('node:child_process');
const crypto = require('node:crypto');
const fs = require('node:fs');
const path = require('node:path');

const source = fs.realpathSync(__dirname);
const root = fs.realpathSync(process.cwd());
const stateFile = path.join(root, '.build_state.json');
const beamCommit = '9b917381ecaa3ebfaadebf4dad872d39ca9a3171';
const configuration = [
  'package.json', 'package-lock.json', 'tsconfig.json', 'tsconfig.test.json'
];
let state = { generated: [] };
let dependenciesDirectory;

function samePath(first, second) {
  if(!first || !second) {
    return false;
  }
  if(path.resolve(first) == path.resolve(second)) {
    return true;
  }
  const firstInfo = fs.statSync(first, { bigint: true, throwIfNoEntry: false });
  const secondInfo = fs.statSync(
    second, { bigint: true, throwIfNoEntry: false });
  return Boolean(firstInfo && secondInfo) && firstInfo.dev == secondInfo.dev &&
    firstInfo.ino == secondInfo.ino;
}

function inspect(filename) {
  return fs.lstatSync(filename, { throwIfNoEntry: false });
}

function save() {
  fs.writeFileSync(stateFile, JSON.stringify(state, null, 2) + '\n');
}

function collect(directory) {
  const result = [];
  if(!inspect(directory)) {
    return result;
  }
  for(const entry of fs.readdirSync(directory, { withFileTypes: true })) {
    const filename = path.join(directory, entry.name);
    result.push(filename);
    if(entry.isDirectory()) {
      result.push(...collect(filename));
    }
  }
  return result.sort();
}

function outputs() {
  const library = path.join(root, 'library');
  const info = inspect(library);
  if(!info) {
    return [];
  }
  if(info.isSymbolicLink() || !info.isDirectory()) {
    throw new Error('The library output path must be a directory, not a link.');
  }
  return [library, ...collect(library)].map(filename =>
    path.relative(root, filename));
}

function record() {
  if(!state.snapshot) {
    return;
  }
  const previous = new Set(state.snapshot);
  state.generated = [...new Set([...state.generated,
    ...outputs().filter(filename => !previous.has(filename))])].filter(
      filename => inspect(path.join(root, filename)));
  delete state.snapshot;
  save();
}

function remove(filenames) {
  for(const relative of [...filenames].sort().reverse()) {
    const filename = path.resolve(root, relative);
    if(!filename.startsWith(root + path.sep)) {
      throw new Error(
        `Generated path is outside the build directory: ${relative}`);
    }
    const info = inspect(filename);
    if(!info) {
      continue;
    }
    const parent = path.relative(root,
      fs.realpathSync(path.dirname(filename)));
    if(parent == '..' || parent.startsWith('..' + path.sep) ||
        path.isAbsolute(parent)) {
      throw new Error(`Generated path crosses a directory link: ${relative}`);
    }
    if(info.isDirectory() && !info.isSymbolicLink()) {
      if(fs.readdirSync(filename).length == 0) {
        fs.rmdirSync(filename);
      }
    } else {
      fs.unlinkSync(filename);
    }
  }
}

function link(name) {
  const target = path.join(source, name);
  const filename = path.join(root, name);
  if(inspect(filename)) {
    if(!samePath(fs.realpathSync(filename), fs.realpathSync(target))) {
      throw new Error(`Refusing to replace the existing ${filename}.`);
    }
    return;
  }
  let type = 'dir';
  if(process.platform == 'win32') {
    type = 'junction';
  }
  fs.symlinkSync(target, filename, type);
  state.generated.push(name);
  save();
}

function copy(name) {
  const filename = path.join(root, name);
  const target = path.join(source, name);
  const content = fs.readFileSync(target);
  const info = inspect(filename);
  if(info) {
    if(samePath(fs.realpathSync(filename), fs.realpathSync(target))) {
      return;
    }
    if(info.isFile() && fs.readFileSync(filename).equals(content)) {
      return;
    }
    if(!info.isFile() || !state.generated.includes(name)) {
      throw new Error(`Refusing to overwrite the existing ${filename}.`);
    }
  } else {
    state.generated.push(name);
    save();
  }
  fs.writeFileSync(filename, content);
}

function run(command, args, directory) {
  const result = spawnSync(command, args, {
    cwd: directory, stdio: 'inherit'
  });
  if(result.error) {
    throw result.error;
  }
  if(result.status != 0) {
    throw new Error(`${command} failed.`);
  }
}

function setup(directory) {
  const repository = path.join(directory, 'Beam');
  if(!inspect(repository)) {
    run('git', ['clone', 'https://github.com/spiretrading/beam', 'Beam'],
      directory);
    run('git', ['checkout', beamCommit], repository);
    return;
  }
  if(!inspect(path.join(repository, '.git'))) {
    throw new Error(`The Beam dependency is not a Git checkout: ${repository}`);
  }
  const result = spawnSync('git',
    ['merge-base', '--is-ancestor', beamCommit, 'HEAD'], {
      cwd: repository, stdio: 'ignore'
    });
  if(result.error) {
    throw result.error;
  }
  if(result.status == 0) {
    return;
  }
  run('git', ['fetch', 'origin'], repository);
  run('git', ['checkout', beamCommit], repository);
}

function configureDependencies() {
  const filename = path.join(root, 'Dependencies');
  const requested = path.resolve(root,
    dependenciesDirectory || state.dependenciesDirectory || 'Dependencies');
  const info = inspect(filename);
  if(!samePath(requested, filename) && info && !info.isSymbolicLink()) {
    throw new Error(`Refusing to replace the existing ${filename}.`);
  }
  fs.mkdirSync(requested, { recursive: true });
  const target = fs.realpathSync(requested);
  if(!samePath(requested, filename)) {
    if(info && !samePath(fs.realpathSync(filename), target)) {
      fs.unlinkSync(filename);
    }
    if(!inspect(filename)) {
      let type = 'dir';
      if(process.platform == 'win32') {
        type = 'junction';
      }
      fs.symlinkSync(target, filename, type);
    }
  }
  state.dependenciesDirectory = target;
  save();
  setup(target);
}

function configure() {
  configureDependencies();
  if(samePath(root, source)) {
    return;
  }
  for(const name of ['source', 'tests']) {
    link(name);
  }
  for(const name of configuration) {
    copy(name);
  }
  copy('LICENSE');
  for(const name of ['build', 'configure']) {
    if(process.platform == 'win32') {
      const filename = path.join(root, name + '.bat');
      if(!inspect(filename)) {
        fs.writeFileSync(filename, '@ECHO OFF\r\nCALL "' +
          path.join(source, name + '.bat') + '" %*\r\n');
      }
    } else {
      const filename = path.join(root, name + '.sh');
      if(!inspect(filename)) {
        fs.symlinkSync(path.join(source, name + '.sh'), filename);
      }
    }
  }
}

function hash(filenames) {
  const digest = crypto.createHash('sha256');
  for(const filename of filenames.sort()) {
    if(!fs.statSync(path.resolve(source, filename)).isFile()) {
      continue;
    }
    digest.update(filename + '\0');
    digest.update(crypto.createHash('sha256').update(
      fs.readFileSync(path.resolve(source, filename))).digest());
  }
  return digest.digest('hex');
}

function npm(args) {
  let command = 'npm';
  if(process.platform == 'win32') {
    command = process.env.ComSpec || 'cmd.exe';
    args = ['/d', '/s', '/c', 'npm ' + args.join(' ')];
  }
  run(command, args, root);
}

function build() {
  configure();
  const beam = fs.realpathSync(path.join(root, 'Dependencies/Beam/WebApi'));
  run(process.execPath, [path.join(beam, 'build.js'), 'build'], beam);
  const dependencies = hash([
    'package.json', 'package-lock.json', path.join(beam, 'package.json')
  ]);
  const manifest = JSON.parse(fs.readFileSync('package.json', 'utf8'));
  const packages = Object.keys({
    ...manifest.dependencies, ...manifest.devDependencies
  });
  const beamLink = path.join(root, 'node_modules/beam');
  if(state.dependencies != dependencies ||
      !inspect('node_modules/.package-lock.json') || packages.some(name =>
        !fs.existsSync(path.join('node_modules', name, 'package.json'))) ||
      !samePath(fs.realpathSync(beamLink), beam)) {
    delete state.dependencies;
    delete state.build;
    save();
    npm(['ci', '--include=dev']);
    state.dependencies = dependencies;
    save();
  }
  const inputs = [...configuration, 'build.js'];
  for(const directory of ['source', 'tests']) {
    inputs.push(...collect(path.join(source, directory)).map(filename =>
      path.relative(source, filename)));
  }
  inputs.push(path.join(beam, '.build_state.json'));
  const current = hash(inputs);
  const generated = state.generated.filter(filename =>
    filename == 'library' || filename.startsWith('library' + path.sep));
  if(state.build == current && inspect('library') &&
      generated.every(filename => inspect(filename))) {
    if(!inspect('mod_time.txt')) {
      fs.writeFileSync('mod_time.txt', current + '\n');
    }
    return;
  }
  delete state.build;
  save();
  fs.rmSync(path.join(root, 'mod_time.txt'), { force: true });
  remove(generated);
  state.snapshot = outputs();
  save();
  try {
    npm(['run', 'build']);
    npm(['test']);
    fs.writeFileSync('mod_time.txt', current + '\n');
    state.build = current;
  } finally {
    record();
  }
}

function clean(reset) {
  delete state.build;
  remove(state.generated);
  if(reset) {
    for(const name of ['node_modules', 'Dependencies']) {
      const filename = path.join(root, name);
      const info = inspect(filename);
      if(info && info.isSymbolicLink()) {
        fs.unlinkSync(filename);
      } else {
        fs.rmSync(filename, { recursive: true, force: true });
      }
    }
  }
  for(const name of ['mod_time.txt', '.build_hash.txt', '.build_state.json']) {
    fs.rmSync(path.join(root, name), { force: true });
  }
}

function main() {
  const args = process.argv.slice(2);
  const command = args.shift();
  let action = '';
  for(let i = 0; i < args.length; ++i) {
    const argument = args[i];
    if(argument == '-DD') {
      ++i;
      if(!args[i] || args[i].startsWith('-')) {
        throw new Error('-DD requires a path argument.');
      }
      dependenciesDirectory = args[i];
    } else if(argument.startsWith('-DD=')) {
      if(argument.length == 4) {
        throw new Error('-DD requires a path argument.');
      }
      dependenciesDirectory = argument.slice(4);
    } else {
      action = argument.toLowerCase();
      if(!['release', 'debug', 'relwithdebinfo', 'minsizerel',
          'clean', 'reset'].includes(action)) {
        throw new Error(`Invalid build argument: ${argument}`);
      }
    }
  }
  if(command == 'setup') {
    setup(root);
    return;
  }
  if(inspect(stateFile)) {
    state = JSON.parse(fs.readFileSync(stateFile, 'utf8'));
    if(state.snapshot) {
      delete state.snapshot;
      save();
    }
  }
  if(command == 'configure') {
    configure();
  } else if(command == 'build') {
    if(action == 'clean' || action == 'reset') {
      clean(action == 'reset');
    } else {
      build();
    }
  } else {
    throw new Error(`Invalid command: ${command}`);
  }
}

try {
  main();
} catch(error) {
  console.error(error.message);
  process.exitCode = 1;
}
