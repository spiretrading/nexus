import * as Nexus from 'nexus';
import * as React from 'react';
import { DisplaySize } from '../../..';
import { FormEntry, TextField } from '.';
import { RolesField } from './roles_field';
import { CommentBox } from '../comment_box';
import { HLine } from '../../../components';
import { VBoxLayout, Padding, HBoxLayout } from '../../../layouts';

interface Properties {

  /** The account's roles. */
  roles: Nexus.AccountRoles;

  /** The account identity to display. */
  identity: Nexus.AccountIdentity;

  /** Determines the layout used to display the page. */
  displaySize: DisplaySize;

  /** Whether the save changes button can be clicked. */
  isSubmitEnabled?: boolean;

  /** The status of the submission. */
  submitStatus?: string;

  /** Whether an error occurred. */
  hasError?: boolean;

  /** Indicates the profile is being submitted. */
  onSubmit?: () => void;

  /** Whether the password button can be clicked. */
  isPasswordSubmitEnabled?: boolean;

  /** The status of the password submission. */
  submitPasswordStatus?: string;

  /** Whether an error occurred submitting the password. */
  hasPasswordError?: boolean;

  /** Indicates the password has been updated. */
  onSubmitPassword?: (password: string) => void;
}

/** Displays an account's profile page. */
export class ProfilePage extends React.Component<Properties> {
  public static readonly defaultProps = {
    isSubmitEnabled: false,
    submitStatus: '',
    hasError: false,
    onSubmit: () => { },
    isPasswordSubmitEnabled: false,
    submitPasswordStatus: '',
    hasPasswordError: false,
    onPasswordSubmit: () => { }
  }

  public render(): JSX.Element {
    const orientation = (() => {
      if (this.props.displaySize === DisplaySize.SMALL) {
        return FormEntry.Orientation.VERTICAL;
      } else {
        return FormEntry.Orientation.HORIZONTAL;
      }
    })();
    return (
      <HBoxLayout width='100%' height='100%'>
        <Padding size={ProfilePage.SIDE_PADDING} />
        <VBoxLayout>
          <div>Last Login</div>
          <div>Accound Information</div>
          <HBoxLayout>
            <div>INSERT PHOTO</div>
             <Padding size={ProfilePage.SIDE_PADDING} />
            <VBoxLayout >
              <FormEntry name='First Name'
                orientation={orientation}>
                <TextField
                  value='Frodo'
                  displaySize={this.props.displaySize}
                  disabled />
              </FormEntry>
              <Padding size={ProfilePage.LINE_PADDING} />
              <HLine color='#E6E6E6' />
              <Padding size={ProfilePage.LINE_PADDING} />
              <FormEntry name='Last Name'
                orientation={orientation}>
                <TextField
                  value='Baggins'
                  displaySize={this.props.displaySize}
                  disabled />
              </FormEntry>
              <Padding size={ProfilePage.LINE_PADDING} />
              <HLine color='#E6E6E6' />
              <Padding size={ProfilePage.LINE_PADDING} />
              <FormEntry name='Username'
                orientation={orientation}>
                <TextField
                  value='frodo_of_the_nine_fingers'
                  displaySize={this.props.displaySize}
                  disabled />
              </FormEntry>
              <Padding size={ProfilePage.LINE_PADDING} />
              <HLine color='#E6E6E6' />
              <Padding size={ProfilePage.LINE_PADDING} />
              <FormEntry name='Role(s)'
                orientation={orientation}>
                <RolesField roles={this.props.roles} />
              </FormEntry>
              <Padding size={ProfilePage.LINE_PADDING} />
              <HLine color='#E6E6E6' />
              <Padding size={ProfilePage.LINE_PADDING} />
              <FormEntry name='Groups(s)'
                orientation={orientation}>
                <TextField
                  value='shire_office'
                  displaySize={this.props.displaySize}
                  disabled />
              </FormEntry>
              <Padding size={ProfilePage.LINE_PADDING} />
              <HLine color='#E6E6E6' />
              <Padding size={ProfilePage.LINE_PADDING} />
              <FormEntry name='Regestration Date'
                orientation={orientation}>
                <TextField
                  value='PUT A GOOD VALUE HERE'
                  displaySize={this.props.displaySize}
                  disabled />
              </FormEntry>
              <Padding size={ProfilePage.LINE_PADDING} />
              <HLine color='#E6E6E6' />
              <Padding size={ProfilePage.LINE_PADDING} />
              <FormEntry name='ID Number'
                orientation={orientation}>
                <TextField
                  value='SR68'
                  displaySize={this.props.displaySize}
                  disabled />
              </FormEntry>
              <Padding size={ProfilePage.LINE_PADDING} />
              <HLine color='#E6E6E6' />
              <Padding size={ProfilePage.LINE_PADDING} />
              <FormEntry name='Email'
                orientation={orientation}>
                <TextField
                  value='frodo@bagend.nz'
                  displaySize={this.props.displaySize}
                  disabled />
              </FormEntry>
              <Padding size={ProfilePage.LINE_PADDING} />
              <HLine color='#E6E6E6' />
              <Padding size={ProfilePage.LINE_PADDING} />
              <FormEntry name='Address'
                orientation={orientation}>
                <TextField
                  value='56 Bag End'
                  displaySize={this.props.displaySize}
                  disabled />
              </FormEntry>
              <Padding size={ProfilePage.LINE_PADDING} />
              <HLine color='#E6E6E6' />
              <Padding size={ProfilePage.LINE_PADDING} />
              <FormEntry name='City'
                orientation={orientation}>
                <TextField
                  value='Hobbiton'
                  displaySize={this.props.displaySize}
                  disabled />
              </FormEntry>
              <Padding size={ProfilePage.LINE_PADDING} />
              <HLine color='#E6E6E6' />
              <Padding size={ProfilePage.LINE_PADDING} />
              <FormEntry name='Province/State'
                orientation={orientation}>
                <TextField
                  value='Westfarthing'
                  displaySize={this.props.displaySize}
                  disabled />
              </FormEntry>
              <Padding size={ProfilePage.LINE_PADDING} />
              <HLine color='#E6E6E6' />
              <Padding size={ProfilePage.LINE_PADDING} />
              <FormEntry name='Country'
                orientation={orientation}>
                <TextField
                  value='Shire'
                  displaySize={this.props.displaySize}
                  disabled />
              </FormEntry>
            </VBoxLayout>
          </HBoxLayout>
          <div>
            <div> User Notes</div>
            <CommentBox comment='boo' />
          </div>
        </VBoxLayout>
        <Padding size={ProfilePage.SIDE_PADDING} />
      </HBoxLayout>);
  }
  private static readonly STYLE = {
    headerStyler: {

    },
    loginInfo: {

    }
  };
  private static readonly LINE_PADDING = '14px';
  private static readonly SIDE_PADDING = '30px';
}
