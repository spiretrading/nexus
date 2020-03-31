import { HBoxLayout, Padding, VBoxLayout } from 'dali';
import * as Nexus from 'nexus';
import * as React from 'react';
import { CommentBox } from './comment_box';
import { SubmitButton } from './submit_button';

interface Properties {

  comment: string;

  /** Determines how a form gets submitted depending on the account's roles. */
  roles: Nexus.AccountRoles;

  /** Whether an error occurred. */
  isError?: boolean;

  /** The status message to display. */
  status?: string;

  /** Determines if the submit button is enabled. */
  isEnabled?: boolean;

  onChange?: (comment: string) => void;

  /** Indicates the form should be submitted. */
  onSubmit?: () => void;
}

/** Displays the components needed to submit an account related form. */
export class SubmissionInput extends React.Component<Properties> {
  public static defaultProps = {
    isEnabled: false,
    isError: false,
    status: '',
    onChange: () => {},
    onSubmit: () => {}
  }

  public render(): JSX.Element {
    const commentBox = (() => {
      if(!this.props.roles.test(Nexus.AccountRoles.Role.ADMINISTRATOR)) {
        return <CommentBox comment={this.props.comment}
          onInput={this.props.onChange}/>;
      }
    })();
    const commentBoxPadding = (() => {
      if(!this.props.roles.test(Nexus.AccountRoles.Role.ADMINISTRATOR)) {
        return <Padding size='30px'/>;
      }
    })();
    const message = (() => {
      if(this.props.status === '') {
        return <div/>;
      } else if(this.props.isError) {
        return <span style={{...SubmissionInput.STYLES.base,
          ...SubmissionInput.STYLES.invalid}}>{this.props.status}</span>;
      } else {
        return <span style={{...SubmissionInput.STYLES.base,
          ...SubmissionInput.STYLES.valid}}>{this.props.status}</span>;
      }
    })();
    return (
      <VBoxLayout width='100%'>
        {commentBox}
        {commentBoxPadding}
        <HBoxLayout width='100%'>
          <Padding size='calc(50% - 123px)'/>
          <SubmitButton isDisabled={!this.props.isEnabled}
            roles={this.props.roles}
            onClick={() => this.props.onSubmit()}/>
          <Padding size='calc(50% - 123px)'/>
        </HBoxLayout>
        <Padding size='18px'/>
        <HBoxLayout width='100%'>
          <Padding/>
          {message}
          <Padding/>
        </HBoxLayout>
      </VBoxLayout>);
  }

  private static readonly STYLES = {
    base: {
      font: '400 14px Roboto',
      width: '100%',
      textAlign: 'center' as 'center'
    },
    valid: {
      color: '#36BB55'
    },
    invalid: {
      color: '#E63F44'
    }
  };
}
