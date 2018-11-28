import * as Nexus from 'nexus';
import * as React from 'react';
import { HBoxLayout, Padding, VBoxLayout } from '../..';
import { CommentBox } from './comment_box';
import { SubmitButton } from './submit_button';

interface Properties {

  /** Determines how a form gets submitted depending on the account's roles. */
  roles: Nexus.AccountRoles;

  /** Whether an error occurred. */
  isError?: boolean;

  /** The status message to display. */
  status?: string;

  /** Whether the submit button is enabled. */
  isEnabled?: boolean;

  /** Indicates the form should be submitted.
   * @param comment - The comment to submit with the form.
   */
  onSubmit?: (comment: string) => void;
}

interface State {
  comment: string;
}

/** Displays the components needed to submit an account related form. */
export class SubmissionBox extends React.Component<Properties, State> {
  public static defaultProps = {
    isEnabled: false,
    isError: false,
    status: '',
    onSubmit: () => {}
  }

  constructor(props: Properties) {
    super(props);
    this.state = {
      comment: ''
    };
    this.onInput = this.onInput.bind(this);
  }

  public render(): JSX.Element {
    const commentBox = (() => {
      if(!this.props.roles.test(Nexus.AccountRoles.Role.ADMINISTRATOR)) {
        return <CommentBox comment={this.state.comment}
          onInput={this.onInput}/>;
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
        return <span style={{...SubmissionBox.STYLES.base,
          ...SubmissionBox.STYLES.invalid}}>{this.props.status}</span>;
      } else {
        return <span style={{...SubmissionBox.STYLES.base,
          ...SubmissionBox.STYLES.valid}}>{this.props.status}</span>;
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
            onClick={() => this.props.onSubmit(this.state.comment)}/>
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

  private onInput(value: string) {
    this.setState({
      comment: value
    });
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
