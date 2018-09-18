import {css, StyleSheet} from 'aphrodite';
import * as React from 'react';

/** Displays the comment box. */
export class CommentBox extends React.Component {
  public render(): JSX.Element {
    return <textarea className={css(CommentBox.STYLE.submissionBox)}
      ref={(ref) => this.textArea = ref}
      placeholder='Leave comment here…'/>;
  }
  
  /** @return The comment currently typed in the CommentBox. */
  public getComment(): string {
    return this.textArea.value;
  }

  private static STYLE = StyleSheet.create({
    submissionBox: {
      boxSizing: 'border-box',
      width: '100%',
      height: '150px',
      border: '1px solid #C8C8C8',
      padding: '10px',
      margin: 0,
      resize: 'none',
      outline: 0,
      ':focus': {
        border: '1px solid #684BC7'
      }
    }
  });
  private textArea: HTMLTextAreaElement;
}
