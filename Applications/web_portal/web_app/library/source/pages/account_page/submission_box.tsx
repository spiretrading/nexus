import {css, StyleSheet} from 'aphrodite';
import * as Nexus from 'nexus';
import * as React from 'react';
import {Center, HBoxLayout, Padding, VBoxLayout} from '../..';
import {CommentBox} from './comment_box';
import {SubmitButton} from './submit_button';

interface Properties {

  /** Determines how a form gets submitted depending on the account's roles. */
  roles: Nexus.AccountRoles;

  /** The status message to display. */
  status?: string;

  /** Indicates the form should be submitted. */
  onClick?: () => void;
}

interface State {
	comment: string;
}

/** Displays the components needed to submit an account related form. */
export class SubmissionBox extends React.Component<Properties, State> {
	constructor(props: Properties) {
		super(props);
		this.state = {
			comment: ''
		};
	}

  public render(): JSX.Element {
    const commentBox = (() => {
      if(!this.props.roles.isSet(Nexus.AccountRoles.Role.ADMINISTRATOR)) {
        return <CommentBox comment={this.state.comment}
        	onChange={this.onCommentChange}/>;
      }
    })();
    const commentBoxPadding = (() => {
     if(!this.props.roles.isSet(Nexus.AccountRoles.Role.ADMINISTRATOR)) {
        return <Padding size='30px'/>;
      }
    })();
    return (
    	<VBoxLayout width='100%'>
        {commentBox}
        {commentBoxPadding}
        <HBoxLayout width='100%'>
          <Padding size='calc(50% - 123px)'/>
          <SubmitButton isDisabled={false}
          	roles={this.props.roles}
            onClick={() => {console.log('clicked')}}/>
          <Padding size='calc(50% - 123px)'/>
        </HBoxLayout>
        <Padding size='18px'/>
        <HBoxLayout width='100%'>
          <Padding/>
          {this.props.status}
          <Padding/>
        </HBoxLayout>
      </VBoxLayout>);
  }

  private onCommentChange(comment: string) {
  	this.setState({comment: comment});
  }
}
