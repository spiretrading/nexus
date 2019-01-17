import * as Beam from 'beam';
import * as Dali from 'dali';
import * as Nexus from 'nexus';
import * as React from 'react';
import { CountrySelectionBox, DisplaySize, HLine, PhotoField } from '../../..';
import { CommentBox } from '../comment_box';
import { ChangePasswordBox, FormEntry, PhotoFieldDisplayMode, RolesField,
  SubmitButton, TextField } from '.';

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

  /** Whether the form can be edited. */
  readonly?: boolean;

  /** Whether the save changes button can be clicked. */
  isSubmitEnabled?: boolean;

  /** The status of the submission. */
  submitStatus?: string;

  /** Whether an error occurred. */
  hasError?: boolean;

  /** Indicates the profile is being submitted. */
  onSubmit?: () => void;

  /** Whether the option to change the password is available. */
  hasPassword?: boolean;

  /** Whether the password button can be clicked. */
  isPasswordSubmitEnabled?: boolean;

  /** The status of the password submission. */
  submitPasswordStatus?: string;

  /** Whether an error occurred submitting the password. */
  hasPasswordError?: boolean;

  /** Indicates the password has been updated. */
  onSubmitPassword?: (password: string) => void;
}

interface State {
  password1: string;
  password2: string;
  newIdentity: Nexus.AccountIdentity;
}

/** Displays an account's profile page. */
export class ProfilePage extends React.Component<Properties, State> {
  public static readonly defaultProps = {
    readonly: false,
    isSubmitEnabled: false,
    submitStatus: '',
    hasError: false,
    onSubmit: () => {},
    hasPassword: false,
    isPasswordSubmitEnabled: false,
    submitPasswordStatus: '',
    hasPasswordError: false,
    onPasswordSubmit: () => {}
  };

  constructor(props: Properties) {
    super(props);
    this.state = {
      password1: '',
      password2: '',
      newIdentity: this.props.identity.clone()
    };
    this.onCommentChange = this.onCommentChange.bind(this);
    this.onCheckPasswordFieldChange =
      this.onCheckPasswordFieldChange.bind(this);
    this.onPasswordFieldChange = this.onPasswordFieldChange.bind(this);
    this.onSubmitPassword = this.onSubmitPassword.bind(this);
    this.onSubmitProfile = this.onSubmitProfile.bind(this);
  }

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
    const commentBoxStyle = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        return ProfilePage.STYLE.stackedStatusBox;
      } else {
        return ProfilePage.STYLE.inlineStatusBox;
      }
    })();
    const commentFooterPaddingSize = (() => {
      if(this.props.readonly) {
        return 0;
      } else {
        return ProfilePage.STANDARD_PADDING;
      }
    })();
    const statusMessageInline = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        return ProfilePage.STYLE.hidden;
      } else if(this.props.hasError) {
        return ProfilePage.STYLE.errorMessage;
      } else {
        return ProfilePage.STYLE.statusMessage;
      }
    })();
    const statusMessageFooter = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        if(this.props.hasError) {
          return ProfilePage.STYLE.errorMessage;
        } else if (this.props.submitStatus) {
          return ProfilePage.STYLE.statusMessage;
        } else {
          return ProfilePage.STYLE.hidden;
        }
      } else {
        return ProfilePage.STYLE.hidden;
      }
    })();
    const changePasswordBox = (() => {
      let passwordButtonEnabled;
      if(this.state.password1 !== ''
          && this.state.password2 !== ''
          && this.props.isPasswordSubmitEnabled) {
        passwordButtonEnabled = true;
      } else {
        passwordButtonEnabled = false;
      }
      if(this.props.hasPassword) {
        return (
          <Dali.VBoxLayout>
            <Dali.Padding size={ProfilePage.STANDARD_PADDING}/>
            <HLine color={ProfilePage.LINE_COLOR}/>
            <Dali.Padding size={ProfilePage.STANDARD_PADDING}/>
            <ChangePasswordBox displaySize={this.props.displaySize}
              hasPasswordError={this.props.hasPasswordError}
              submitPasswordStatus={this.props.submitPasswordStatus}
              isPasswordSubmitEnabled={passwordButtonEnabled}
              onSubmitPassword={this.onSubmitPassword}
              password1={this.state.password1}
              password2={this.state.password2}
              password1OnChange={this.onPasswordFieldChange}
              password2OnChange={this.onCheckPasswordFieldChange}/>
          </Dali.VBoxLayout>);
      } else {
        return null;
      }
    })();
    const commentBoxButtonStyle = (() => {
      if(this.props.readonly) {
        return ProfilePage.STYLE.hidden;
      } else  {
        return null;
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
            <Dali.VBoxLayout style={null}>
              <div style={ProfilePage.STYLE.headerStyler}>
                User Notes
              </div>
              <Dali.Padding size={ProfilePage.STANDARD_PADDING}/>
              <CommentBox comment={this.state.newIdentity.userNotes}
                readonly={this.props.readonly}
                onInput={this.onCommentChange}/>
              <Dali.Padding size={commentFooterPaddingSize}/>
              <div style={{...commentBoxStyle, ...commentBoxButtonStyle}}>
                <div style={ProfilePage.STYLE.filler}/>
                <div style={{ ...commentBoxStyle, ...statusMessageInline}}>
                  {this.props.submitStatus}
                  <div style=
                    {ProfilePage.STYLE.buttonPadding}/>
                </div>
                <SubmitButton label='Save Changes'
                  displaySize={this.props.displaySize}
                  isSubmitEnabled={this.props.isSubmitEnabled}
                  onClick={this.onSubmitProfile}/>
                <div style={statusMessageFooter}>
                  <div style={ProfilePage.STYLE.smallPadding}/>
                  {this.props.submitStatus}
                </div>
              </div>
            </Dali.VBoxLayout>
            {changePasswordBox}
            <Dali.Padding size={ProfilePage.BOTTOM_PADDING}/>
          </Dali.VBoxLayout>
        </div>
        <div style={ProfilePage.STYLE.pagePadding}/>
      </div>);
  }

  private onCommentChange(newComment: string) {
    const testIdentity = this.state.newIdentity;
    testIdentity.userNotes = newComment;
    this.setState({newIdentity: testIdentity});
  }

  private onPasswordFieldChange(testPassword: string) {
    this.setState({password1: testPassword});
  }

  private onCheckPasswordFieldChange(testConfirmPassword: string) {
    this.setState({password2: testConfirmPassword});
  }

  private onSubmitProfile() {
    this.props.onSubmit();
  }

  private onSubmitPassword() {
    if(this.state.password1 === this.state.password2) {
      console.log('Passwords match!');
      this.props.onSubmitPassword(this.state.password1);
    } else {
      console.log('Passwords do not match!');
    }
    this.setState({
      password1: '',
      password2: ''
    });
    console.log('Passsword was submitted.');
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
    buttonPadding: {
      height: '30px',
      width: '30px'
    }
  };
  private static readonly LINE_PADDING = '14px';
  private static readonly STANDARD_PADDING = '30px';
  private static readonly BOTTOM_PADDING = '60px';
  private static readonly LINE_COLOR = '#E6E6E6';
}
