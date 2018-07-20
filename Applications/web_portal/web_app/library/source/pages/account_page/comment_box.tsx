import {css, StyleSheet} from 'aphrodite';
import * as React from 'react';

interface Properties {
	onChange: (comment: string) => void;
	comment: string;
}

/** Displays the comment box. */
export class CommentBox extends React.Component<Properties> {
  public render(): JSX.Element {
    return <textarea className={css(CommentBox.STYLE.submissionBox)}
      value={this.props.comment} placeholder='Leave comment here…'
      onChange={(event) => this.props.onChange(event.currentTarget.value)}/>
  }

  private static STYLE = StyleSheet.create({
    submissionBox: {
      boxSizing: 'border-box',
      width: '100%',
      height: '130px',
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
