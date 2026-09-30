/** Reads nonempty entries from a CSV file, including quoted fields.
 * @param text - The file contents.
 * @return The trimmed entries in row order.
 */
export function parseCsv(text: string): string[] {
  const entries: string[] = [];
  let field = '';
  let quoted = false;
  let closed = false;
  const append = () => {
    if(field.trim()) {
      entries.push(field.trim());
    }
    field = '';
    closed = false;
  };
  for(let i = 0; i < text.length; ++i) {
    const character = text[i];
    if(quoted) {
      if(character !== '"') {
        field += character;
      } else if(text[i + 1] === '"') {
        field += '"';
        ++i;
      } else {
        quoted = false;
        closed = true;
      }
    } else if(character === ',' || character === '\n' || character === '\r') {
      append();
    } else if(character === '"' && !field.trim() && !closed) {
      field = '';
      quoted = true;
    } else if(character === '"' || closed && character.trim()) {
      throw new Error('Invalid CSV quoting.');
    } else {
      field += character;
    }
  }
  if(quoted) {
    throw new Error('An entry has an unclosed quote.');
  }
  append();
  return entries;
}
