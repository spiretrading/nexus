import DOMPurify from 'dompurify';
import { Marked } from 'marked';
import { create } from '../report_viewer/text';

function render(text) {
  const content = document.createElement('article');
  const html = MARKDOWN.parse(text);
  content.appendChild(DOMPurify.sanitize(html, {
    ALLOWED_TAGS: ['a', 'blockquote', 'br', 'code', 'del', 'div', 'em',
      'h1', 'h2', 'h3', 'h4', 'h5', 'h6', 'hr', 'img', 'li', 'ol', 'p',
      'pre', 's', 'span', 'strong', 'sub', 'sup', 'table', 'tbody', 'td',
      'th', 'thead', 'tr', 'ul'],
    ALLOWED_ATTR: ['href', 'title', 'src', 'alt', 'start', 'align'],
    ALLOW_DATA_ATTR: false,
    ALLOW_ARIA_ATTR: false,
    RETURN_DOM_FRAGMENT: true
  }));
  for(const link of content.querySelectorAll('a[href]')) {
    link.target = '_blank';
    link.rel = 'noopener noreferrer';
  }
  return content;
}

window.spireMarkdownViewerReady = Promise.resolve({
  create(container, file) {
    return create(container, file, render);
  }
});

const MARKDOWN = new Marked({
  async: false,
  gfm: true,
  renderer: {
    checkbox({checked}) {
      if(checked) {
        return '[x] ';
      }
      return '[ ] ';
    }
  }
});
