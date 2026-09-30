import * as React from 'react';

/** Highlights literal, case-insensitive query matches in report text. */
export function highlightText(text: string, query: string): React.ReactNode {
  if(!query) {
    return text;
  }
  const escaped = query.replace(/[.*+?^${}()|[\]\\]/g, '\\$&');
  return text.split(new RegExp(`(${escaped})`, 'gi')).map((part, index) => {
    if(index % 2 === 1) {
      return <mark key={index}
        style={{backgroundColor: '#FFF7C4', color: 'inherit'}}>{part}</mark>;
    }
    return part;
  });
}
