const crypto = require('node:crypto');
const fs = require('node:fs');
const path = require('node:path');

async function main() {
  const [cacheDirectory, outputDirectory] = process.argv.slice(2);
  if(!cacheDirectory || !outputDirectory) {
    throw new Error('PDF.js cache and output directories are required.');
  }
  const extract = require(path.join(
    process.cwd(), 'node_modules/extract-zip'));
  const version = '6.5.438';
  const expected =
    'fcbe9ba21d9911843e3502adc178a7f549a524a4055f887bedc942e5490c6ba1';
  const filename = `pdfjs-${version}-dist.zip`;
  const archive = path.resolve(cacheDirectory, filename);
  const matches = buffer =>
    crypto.createHash('sha256').update(buffer).digest('hex') == expected;
  if(!fs.existsSync(archive) || !matches(fs.readFileSync(archive))) {
    const url = 'https://github.com/mozilla/pdf.js/releases/download/' +
      `v${version}/${filename}`;
    const response = await fetch(url, {signal: AbortSignal.timeout(300000)});
    if(!response.ok) {
      throw new Error(`PDF.js download failed with HTTP ${response.status}.`);
    }
    const buffer = Buffer.from(await response.arrayBuffer());
    if(!matches(buffer)) {
      throw new Error('PDF.js archive checksum does not match.');
    }
    fs.mkdirSync(cacheDirectory, {recursive: true});
    const temporary = archive + '.' + crypto.randomUUID() + '.tmp';
    try {
      fs.writeFileSync(temporary, buffer, {flag: 'wx'});
      fs.renameSync(temporary, archive);
    } finally {
      fs.rmSync(temporary, {force: true});
    }
  }
  await extract(archive, {dir: path.resolve(outputDirectory)});
}

main().catch(error => {
  console.error(error.message);
  process.exitCode = 1;
});
