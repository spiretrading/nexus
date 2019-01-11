import * as Dali from 'dali';
import * as React from 'react';
import { CommentBox } from '../comment_box';
import { DisplaySize } from '../../..';
import { SubmitButton } from '.';

interface Properties  {
  displaySize: DisplaySize;
  isSubmitEnabled: boolean;
  submitStatus: string;
  hasError: boolean;
  onSubmit: () => void;
}

export class SubmitCommentBox extends React.Component<Properties > {
  public static readonly defaultProps = {
    isSubmitEnabled: false,
    submitStatus: '',
    hasError: false,
    onSubmit: () => {}
  };

  public render(): JSX.Element {
    const boxStyle = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        return SubmitCommentBox.STYLE.stackedStatusBox;
      } else {
        return SubmitCommentBox.STYLE.inlineStatusBox;
      }
    })();
    const statusMessageInline = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        return SubmitCommentBox.STYLE.hidden;
      } else {
        if(this.props.hasError) {
          return SubmitCommentBox.STYLE.errorMessage;
        } else {
          return SubmitCommentBox.STYLE.statusMessage;
        }
      }
    })();
    const statusMessageFooter = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        if(this.props.hasError) {
          return SubmitCommentBox.STYLE.errorMessage;
        } else {
          return SubmitCommentBox.STYLE.statusMessage;
        }
      } else {
        return SubmitCommentBox.STYLE.hidden;
      }
    })();
    return (
      <Dali.VBoxLayout>
        <div style={SubmitCommentBox.STYLE.headerStyler}>User Notes</div>
        <Dali.Padding size={SubmitCommentBox.STANDARD_PADDING}/>
        <CommentBox comment=''/>
        <Dali.Padding size={SubmitCommentBox.STANDARD_PADDING}/>
        <div style={boxStyle}>
          <div style={SubmitCommentBox.STYLE.filler}/>
          <div style={{ ...boxStyle, ...statusMessageInline}}>
            {this.props.submitStatus}
            <div style={SubmitCommentBox.STYLE.passwordButtonPadding}/>
          </div>
          <SubmitButton label='Save Changes'
            displaySize={this.props.displaySize}/>
          <div style={statusMessageFooter}>
            <div style={SubmitCommentBox.STYLE.smallPadding}/>
            {this.props.submitStatus}
          </div>
        </div>
      </Dali.VBoxLayout>);
  }
  private static readonly STYLE = {
    hidden: {
      opacity: 0,
      visibility: 'hidden' as 'hidden',
      display: 'none' as 'none'
    },
    headerStyler: {
      color: '#4B23A0',
      font: '500 14px Roboto',
      wdith: '100%'
    },
    errorMessage: {
      color: '#E63F44',
      font: '400 14px Roboto'
    },
    statusMessage: {
      color: '#36BB55',
      font: '400 14px Roboto'
    },
    filler: {
      flexGrow: 1
    },
    smallPadding: {
      width: '100%',
      height: '18px'
    },
    mediumPadding: {
      width: '100%',
      height: '30px'
    },
    inlineStatusBox: {
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      flexWrap: 'nowrap' as 'nowrap',
      alignItems: 'center' as 'center'
    },
    stackedStatusBox: {
      display: 'flex' as 'flex',
      flexDirection: 'column' as 'column',
      alignItems: 'center' as 'center',
      justifyContent: 'center' as 'center'
    },
    passwordButtonPadding: {
      height: '30px',
      width: '30px'
    }
  };
  private static readonly STANDARD_PADDING = '30px';
}
