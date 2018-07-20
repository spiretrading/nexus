import {css, StyleSheet} from 'aphrodite';
import * as React from 'react';

interface Properties {
}

/** Displays the comment box. */
export class CommentBox extends React.Component<Properties> {
	constructor(props: Properties) {
		super(props);
	}
  public render(): JSX.Element {
    return <textarea className={css(CommentBox.STYLE.submissionBox)}
    	ref={(ref) => this.textArea = ref}
      placeholder='Leave comment here…'/>;
  }
  
  public getComment() {
  	return this.textArea.value;
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
  private textArea: HTMLTextAreaElement;
}
