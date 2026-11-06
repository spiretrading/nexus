import { create, preformatted } from '../report_viewer/text';

function format(text) {
  JSON.parse(text);
  const tokens =
    text.match(/"(?:[^"\\]|\\[\s\S])*"|[^\s{}\[\],:]+|[{}\[\],:]/g);
  const output = [];
  let depth = 0;
  const newline = () => output.push('\n', '  '.repeat(depth));
  for(let i = 0; i < tokens.length; ++i) {
    const token = tokens[i];
    if(token === '{' || token === '[') {
      output.push(token);
      ++depth;
      if(tokens[i + 1] !== '}' && tokens[i + 1] !== ']') {
        newline();
      }
    } else if(token === '}' || token === ']') {
      --depth;
      if(tokens[i - 1] !== '{' && tokens[i - 1] !== '[') {
        newline();
      }
      output.push(token);
    } else if(token === ',') {
      output.push(token);
      newline();
    } else if(token === ':') {
      output.push(': ');
    } else {
      output.push(token);
    }
  }
  return preformatted(output.join(''));
}

window.spireJsonViewerReady = Promise.resolve({
  create(container, file) {
    return create(container, file, format);
  }
});
