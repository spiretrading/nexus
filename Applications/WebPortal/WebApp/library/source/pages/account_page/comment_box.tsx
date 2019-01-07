import { css, StyleSheet } from 'aphrodite';
import * as React from 'react';

interface Properties {

  /** The text to display in the comment box. */
  comment: string;

  /** The callback function that updates the comment. */
  onInput?: (value: string) => void;
}

/** Displays the comment box. */
export class CommentBox extends React.Component<Properties> {
  public render(): JSX.Element {
    return <textarea className={css(CommentBox.STYLE.submissionBox)}
      value={this.props.comment}
      onChange={(event: React.ChangeEvent<HTMLTextAreaElement>) => {
        this.props.onInput(event.target.value);
      }}
      placeholder='Leave comment here…'/>;
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
}
