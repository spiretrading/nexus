import * as Nexus from 'nexus';
import * as React from 'react';
import { DisplaySize, PhotoField } from '../../..';
import { FormEntry, TextField } from '.';
import { RolesField } from './roles_field';
import { CommentBox } from '../comment_box';
import { HLine } from '../../../components';
import { VBoxLayout, Padding, HBoxLayout } from '../../../layouts';
import { DisplayMode } from './photo_field';

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
    const contentWidth = (() => {
      switch (this.props.displaySize) {
        case DisplaySize.SMALL:
          return null;
        case DisplaySize.MEDIUM:
          return '732px';
        case DisplaySize.LARGE:
          return '1000px';
      }
    })();
    const sidePanelPhoto = (() => {
      if (this.props.displaySize === DisplaySize.SMALL) {
        return null;
      } else {
        return (<PhotoField
          displaySize={this.props.displaySize}
          displayMode={DisplayMode.DISPLAY}
          imageSource={this.props.identity.photoId}
          scaling={1} />);
      }
    })();
    const sidePhotoPadding = (() => {
      switch (this.props.displaySize) {
        case DisplaySize.SMALL:
          return 0;
        case DisplaySize.MEDIUM:
          return '30px';
        case DisplaySize.LARGE:
          return '100px';
      }
    })();
    const topPanelPhoto = (() => {
      if (this.props.displaySize === DisplaySize.SMALL) {
        return (<PhotoField
          displaySize={this.props.displaySize}
          displayMode={DisplayMode.DISPLAY}
          imageSource={this.props.identity.photoId}
          scaling={1} />);
      } else {
        return null;
      }
    })();
    return (
      <div style={ProfilePage.STYLE.page}>
        <div style={ProfilePage.STYLE.fixedSizePadding}/>
        <VBoxLayout width={contentWidth}>
          <div style={ProfilePage.STYLE.loginInfo}>Last Login</div>
          <Padding size='30px' />
          <div style={ProfilePage.STYLE.headerStyler}>Accound Information</div>
          <Padding size='30px' />
          <HBoxLayout width={contentWidth}>
            {sidePanelPhoto}
            <Padding size={sidePhotoPadding} />
            <VBoxLayout width='100%' >
              {topPanelPhoto}
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

          <div style={ProfilePage.STYLE.headerStyler}> User Notes</div>
          <CommentBox comment='boo' />

        </VBoxLayout>
        <div style={ProfilePage.STYLE.fixedSizePadding}/>
      </div>);
  }
  private static readonly STYLE = {
    page: {
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row'
    },
    
    headerStyler: {
      color: '#4B23A0',
      font: '500 14px Roboto',
      wdith: '100%'
    },
    loginInfo: {
      color: '#000000',
      font: '400 14px Roboto'
    },
    hidden: {
      opacity: 0,
      visibility: 'hidden' as 'hidden',
      display: 'none' as 'none'
    },
    filler: {
      flexGrow: 1
    },
    fixedSizePadding: {
      width: '30px'
    }
  };
  private static readonly LINE_PADDING = '14px';
  private static readonly SIDE_PADDING = '30px';
}
