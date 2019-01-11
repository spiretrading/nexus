import * as Beam from 'beam';
import * as Dali from 'dali';
import * as Nexus from 'nexus';
import * as React from 'react';
import { CountrySelectionBox, DisplaySize, HLine, PhotoField } from '../../..';
import { ChangePasswordBox, FormEntry, PhotoFieldDisplayMode, RolesField,
  TextField, SubmitCommentBox, SubmitButton } from '.';

interface Properties {

  /** The account name. */
  account: Beam.DirectoryEntry;

  /** The account's roles. */
  roles: Nexus.AccountRoles;

  /** The account identity to display. */
  identity: Nexus.AccountIdentity;

  /** The name of the group the account belongs to. */
  group: Beam.DirectoryEntry;

  /** The database of all available countries. */
  countryDatabase: Nexus.CountryDatabase;

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
    onSubmit: () => {},
    isPasswordSubmitEnabled: false,
    submitPasswordStatus: '',
    hasPasswordError: false,
    onPasswordSubmit: () => {}
  };

  public render(): JSX.Element {
    const contentWidth = (() => {
      switch(this.props.displaySize) {
        case DisplaySize.SMALL:
          return ProfilePage.STYLE.contentSmall;
        case DisplaySize.MEDIUM:
          return ProfilePage.STYLE.contentMedium;
        case DisplaySize.LARGE:
          return ProfilePage.STYLE.contentLarge;
      }
    })();
    const sidePanelPhoto = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        return null;
      } else {
        return (
          <PhotoField
            displaySize={this.props.displaySize}
            displayMode={PhotoFieldDisplayMode.DISPLAY}
            imageSource={this.props.identity.photoId}
            scaling={1}/>);
      }
    })();
    const sidePanelPhotoPadding = (() => {
      switch(this.props.displaySize) {
        case DisplaySize.SMALL:
          return 0;
        case DisplaySize.MEDIUM:
          return '30px';
        case DisplaySize.LARGE:
          return '100px';
      }
    })();
    const topPanelPhoto = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        return (
          <Dali.VBoxLayout>
            <PhotoField
              displaySize={this.props.displaySize}
              displayMode={PhotoFieldDisplayMode.DISPLAY}
              imageSource={this.props.identity.photoId}
              scaling={1}/>
            <Dali.Padding size='30px'/>
          </Dali.VBoxLayout>);
      } else {
        return null;
      }
    })();
    const formFooter = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        return <HLine color={ProfilePage.LINE_COLOR}/>;
      } else {
        return (null);
      }
    })();
    const formFooterPaddingSize = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        return ProfilePage.STANDARD_PADDING;
      } else {
        return 0;
      }
    })();
    return (
      <div style={ProfilePage.STYLE.page}>
        <div style={ProfilePage.STYLE.pagePadding}/>
        <div style={contentWidth}>
          <Dali.VBoxLayout width='100%'>
            <Dali.Padding size='18px'/>
            <div style={ProfilePage.STYLE.lastLoginBox}>
              {this.props.identity.lastLoginTime.toString()}
            </div>
            <Dali.Padding size={ProfilePage.STANDARD_PADDING}/>
            <div style={ProfilePage.STYLE.headerStyler}>
              Account Information
            </div>
            <Dali.Padding size={ProfilePage.STANDARD_PADDING}/>
            <Dali.HBoxLayout>
              {sidePanelPhoto}
              <Dali.Padding size={sidePanelPhotoPadding}/>
              <Dali.VBoxLayout width='100%'>
                {topPanelPhoto}
                <FormEntry name='First Name'
                    displaySize={this.props.displaySize}>
                  <TextField
                    value={this.props.identity.firstName}
                    displaySize={this.props.displaySize}
                    disabled/>
                </FormEntry>
                <Dali.Padding size={ProfilePage.LINE_PADDING}/>
                <HLine color={ProfilePage.LINE_COLOR}/>
                <Dali.Padding size={ProfilePage.LINE_PADDING}/>
                <FormEntry name='Last Name'
                    displaySize={this.props.displaySize}>
                  <TextField
                    value={this.props.identity.lastName}
                    displaySize={this.props.displaySize}
                    disabled/>
                </FormEntry>
                <Dali.Padding size={ProfilePage.LINE_PADDING}/>
                <HLine color={ProfilePage.LINE_COLOR}/>
                <Dali.Padding size={ProfilePage.LINE_PADDING}/>
                <FormEntry name='Username'
                    displaySize={this.props.displaySize}>
                  <TextField
                    value={this.props.account.name.toString()}
                    displaySize={this.props.displaySize}
                    disabled/>
                </FormEntry>
                <Dali.Padding size={ProfilePage.LINE_PADDING}/>
                <HLine color={ProfilePage.LINE_COLOR}/>
                <Dali.Padding size={ProfilePage.LINE_PADDING}/>
                <FormEntry name='Role(s)'
                    displaySize={this.props.displaySize}>
                  <div style={ProfilePage.STYLE.rolesWrapper}>
                    <RolesField roles={this.props.roles}/>
                  </div>
                </FormEntry>
                <Dali.Padding size={ProfilePage.LINE_PADDING}/>
                <HLine color={ProfilePage.LINE_COLOR}/>
                <Dali.Padding size={ProfilePage.LINE_PADDING}/>
                <FormEntry name='Groups(s)'
                    displaySize={this.props.displaySize}>
                  <TextField value={this.props.group.name.toString()}
                    displaySize={this.props.displaySize}
                    disabled/>
                </FormEntry>
                <Dali.Padding size={ProfilePage.LINE_PADDING}/>
                <HLine color={ProfilePage.LINE_COLOR}/>
                <Dali.Padding size={ProfilePage.LINE_PADDING}/>
                <FormEntry name='Registration Date'
                    displaySize={this.props.displaySize}>
                  <TextField displaySize={this.props.displaySize}
                    value={this.props.identity.
                      registrationTime.toString()}
                    disabled/>
                </FormEntry>
                <Dali.Padding size={ProfilePage.LINE_PADDING}/>
                <HLine color={ProfilePage.LINE_COLOR}/>
                <Dali.Padding size={ProfilePage.LINE_PADDING}/>
                <FormEntry name='ID Number'
                    displaySize={this.props.displaySize}>
                  <TextField
                    value={this.props.account.id.toString()}
                    displaySize={this.props.displaySize}
                    disabled/>
                </FormEntry>
                <Dali.Padding size={ProfilePage.LINE_PADDING}/>
                <HLine color={ProfilePage.LINE_COLOR}/>
                <Dali.Padding size={ProfilePage.LINE_PADDING}/>
                <FormEntry name='Email'
                    displaySize={this.props.displaySize}>
                  <TextField
                    value={this.props.identity.emailAddress}
                    displaySize={this.props.displaySize}
                    disabled/>
                </FormEntry>
                <Dali.Padding size={ProfilePage.LINE_PADDING}/>
                <HLine color={ProfilePage.LINE_COLOR}/>
                <Dali.Padding size={ProfilePage.LINE_PADDING}/>
                <FormEntry name='Address'
                    displaySize={this.props.displaySize}>
                  <TextField
                    value={this.props.identity.firstName}
                    displaySize={this.props.displaySize}
                    disabled/>
                </FormEntry>
                <Dali.Padding size={ProfilePage.LINE_PADDING}/>
                <HLine color={ProfilePage.LINE_COLOR}/>
                <Dali.Padding size={ProfilePage.LINE_PADDING}/>
                <FormEntry name='City'
                    displaySize={this.props.displaySize}>
                  <TextField
                    value={this.props.identity.city}
                    displaySize={this.props.displaySize}
                    disabled/>
                </FormEntry>
                <Dali.Padding size={ProfilePage.LINE_PADDING}/>
                <HLine color={ProfilePage.LINE_COLOR}/>
                <Dali.Padding size={ProfilePage.LINE_PADDING}/>
                <FormEntry name='Province/State'
                    displaySize={this.props.displaySize}>
                  <TextField
                    value={this.props.identity.province}
                    displaySize={this.props.displaySize}
                    disabled/>
                </FormEntry>
                <Dali.Padding size={ProfilePage.LINE_PADDING}/>
                <HLine color={ProfilePage.LINE_COLOR}/>
                <Dali.Padding size={ProfilePage.LINE_PADDING}/>
                <FormEntry name='Country'
                    displaySize={this.props.displaySize}>
                  <CountrySelectionBox
                    readonly
                    displaySize={this.props.displaySize}
                    value={this.props.identity.country}
                    countryDatabase={this.props.countryDatabase}/>
                </FormEntry>
                <Dali.Padding size={ProfilePage.STANDARD_PADDING}/>
                {formFooter}
                <Dali.Padding size={formFooterPaddingSize}/>
              </Dali.VBoxLayout>
            </Dali.HBoxLayout>
            <SubmitCommentBox displaySize={this.props.displaySize}
              hasError={this.props.hasError}
              submitStatus={this.props.submitStatus}/>
            <Dali.Padding size={ProfilePage.STANDARD_PADDING}/>
            <HLine color={ProfilePage.LINE_COLOR}/>
            <Dali.Padding size={ProfilePage.STANDARD_PADDING}/>
            <ChangePasswordBox displaySize={this.props.displaySize}
              hasPasswordError={this.props.hasPasswordError}
              submitPasswordStatus={this.props.submitPasswordStatus}/>
            <Dali.Padding size={ProfilePage.BOTTOM_PADDING}/>
          </Dali.VBoxLayout>
        </div>
        <div style={ProfilePage.STYLE.pagePadding}/>
      </div>);
  }

  private static readonly STYLE = {
    page: {
      height: '100%',
      width: '100%',
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      justifyContent: 'space-between' as 'space-between',
      overflowY: 'auto' as 'auto'
    },
    hidden: {
      opacity: 0,
      visibility: 'hidden' as 'hidden',
      display: 'none' as 'none'
    },
    headerStyler: {
      color: '#4B23A0',
      font: '500 14px Roboto',
      width: '100%'
    },
    lastLoginBox: {
      color: '#000000',
      font: '400 14px Roboto'
    },
    contentSmall: {
      flexGrow: 1,
      maxWidth: '424px'
    },
    contentMedium: {
      width: '732px'
    },
    contentLarge: {
      width: '1000px'
    },
    pagePadding: {
      width: '30px'
    },
    rolesWrapper: {
      marginLeft: '11px',
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      height: '34px',
      justifyContent: 'flex-start',
      alignItems: 'center'
    }
  };
  private static readonly LINE_PADDING = '14px';
  private static readonly STANDARD_PADDING = '30px';
  private static readonly BOTTOM_PADDING = '60px';
  private static readonly LINE_COLOR = '#E6E6E6';
}
