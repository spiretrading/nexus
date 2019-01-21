import { css, StyleSheet } from 'aphrodite';
import * as React from 'react';

interface Properties {

  /** Whether the comment box can be edited. */
  readonly?: boolean;

  /** The text to display in the comment box. */
  comment: string;

  /** The callback function that updates the comment. */
  onInput?: (value: string) => void;
}

/** Displays the comment box. */
export class CommentBox extends React.Component<Properties> {

  public render(): JSX.Element {
    const placeholderText = (() => {
      if(this.props.readonly) {
        return '';
      } else {
        return 'Leave comment here…';
      }
    })();
    return <textarea className={css(CommentBox.STYLE.submissionBox)}
      value={this.props.comment}
      readOnly={this.props.readonly}
      onChange={(event: React.ChangeEvent<HTMLTextAreaElement>) => {
        this.props.onInput(event.target.value);
      }}
      placeholder={placeholderText}/>;
  }

  private static STYLE = StyleSheet.create({
    submissionBox: {
      boxSizing: 'border-box',
      font: '400 14px Roboto',
      width: '100%',
      height: '150px',
      border: '1px solid #C8C8C8',
      borderRadius: '1px',
      padding: '10px',
      margin: 0,
      resize: 'none',
      outline: 0,
      color: '#000000',
      ':focus': {
        border: '1px solid #684BC7'
      },
      ':-moz-read-only': {
        border: '1px solid #C8C8C8',
        color: 'transparent',
        textShadow: '0 0 0 #000000'
      },
      ':read-only': {
        border: '1px solid #C8C8C8',
        color: 'transparent',
        textShadow: '0 0 0 #000000'
      }
    }
  });
}
