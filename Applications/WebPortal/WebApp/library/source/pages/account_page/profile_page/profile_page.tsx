import * as Beam from 'beam';
import * as Dali from 'dali';
import * as Nexus from 'nexus';
import * as React from 'react';
import { CountrySelectionField, DisplaySize, HLine, PageWrapper, PhotoField } 
  from '../../..';
import { CommentBox } from '..';
import { ChangePasswordBox } from './change_password_box';
import { FormEntry } from './form_entry';
import { PhotoFieldDisplayMode } from './photo_field';
import { RolesField } from './roles_field';
import { SubmitButton } from './submit_button';
import { TextField } from './text_field';

interface Properties {

  /** The account name. */
  account: Beam.DirectoryEntry;

  /** The account's roles. */
  roles: Nexus.AccountRoles;

  /** The account identity to display. */
  identity: Nexus.AccountIdentity;

  /** The groups the account belongs to. */
  groups: Beam.DirectoryEntry[];

  /** The database of all available countries. */
  countryDatabase: Nexus.CountryDatabase;

  /** Determines the layout used to display the page. */
  displaySize: DisplaySize;

  /** Whether the form can be edited. */
  readonly?: boolean;

  /** The status of the submission. */
  submitStatus?: string;

  /** Whether an error occurred. */
  hasError?: boolean;

  /** Indicates the profile is being submitted. */
  onSubmit?: (roles: Nexus.AccountRoles, identity: Nexus.AccountIdentity) => void;

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
  hasLocalPasswordError: boolean;
  localPasswordMessage: string;
  isPasswordChanged: boolean;
  newRoles: Nexus.AccountRoles;
  newIdentity: Nexus.AccountIdentity;
  isProfileChanged: boolean;
}

/** Displays an account's profile page. */
export class ProfilePage extends React.Component<Properties, State> {
  public static readonly defaultProps = {
    readonly: false,
    submitStatus: '',
    hasError: false,
    onSubmit: () => {},
    hasPassword: false,
    submitPasswordStatus: '',
    hasPasswordError: false,
    onPasswordSubmit: () => {}
  };

  constructor(props: Properties) {
    super(props);
    this.state = {
      password1: '',
      password2: '',
      hasLocalPasswordError: false,
      localPasswordMessage: '',
      isPasswordChanged: false,
      newRoles: this.props.roles.clone(),
      newIdentity: this.props.identity.clone(),
      isProfileChanged: false
    };
    this.onFirstNameChange = this.onFirstNameChange.bind(this);
    this.onLastNameChange = this.onLastNameChange.bind(this);
    this.onEmailChange = this.onEmailChange.bind(this);
    this.onAddressChange = this.onAddressChange.bind(this);
    this.onCityChange = this.onCityChange.bind(this);
    this.onProvinceChange = this.onProvinceChange.bind(this);
    this.onCountryChange = this.onCountryChange.bind(this);
    this.onRolesChange = this.onRolesChange.bind(this);
    this.onCommentChange = this.onCommentChange.bind(this);
    this.onSubmitProfile = this.onSubmitProfile.bind(this);
    this.onPassword1Change = this.onPassword1Change.bind(this);
    this.onPassword2Change = this.onPassword2Change.bind(this);
    this.onSubmitPassword = this.onSubmitPassword.bind(this);
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
            newImageSource={null}
            newScaling={1}
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
              newImageSource={null}
              newScaling={1}
              scaling={1}/>
            <Dali.Padding size={ProfilePage.STANDARD_PADDING}/>
          </Dali.VBoxLayout>);
      } else {
        return null;
      }
    })();
    const groupsList = (() => {
      const list = [];
      for(const group of this.props.groups) {
        list.push(
          <TextField value={group.name.toString()}
            displaySize={this.props.displaySize}
            key={group.id}
            readonly/>);
        list.push(
          <Dali.Padding
            key={'padding' + group.id}
            size='5px'/>);
      }
      return list;
    })();
    const countryBox = (() => {
      if(this.props.readonly) {
        return (
          <TextField
            value={this.props.countryDatabase.fromCode(
              this.props.identity.country).name}
            displaySize={this.props.displaySize}
            readonly={this.props.readonly}/>);
      } else {
        return (
          <CountrySelectionField
            displaySize={this.props.displaySize}
            value={this.state.newIdentity.country}
            onChange={this.onCountryChange}
            countryDatabase={this.props.countryDatabase}/>);
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
        } else if(this.props.submitStatus) {
          return ProfilePage.STYLE.statusMessage;
        } else {
          return ProfilePage.STYLE.hidden;
        }
      } else {
        return ProfilePage.STYLE.hidden;
      }
    })();
    const changePasswordBox = (() => {
      const passwordButtonEnabled = this.state.password1 !== '' &&
        this.state.password2 !== '';
      const status = (() => {
        if(this.state.isPasswordChanged) {
          return '';
        } else if(this.state.localPasswordMessage !== '') {
          return this.state.localPasswordMessage;
        } else {
          return this.props.submitPasswordStatus;
        }})();
      return (
        <Dali.VBoxLayout>
          <Dali.Padding size={ProfilePage.STANDARD_PADDING}/>
          <HLine color={ProfilePage.LINE_COLOR}/>
          <Dali.Padding size={ProfilePage.STANDARD_PADDING}/>
          <ChangePasswordBox displaySize={this.props.displaySize}
            hasPasswordError={this.props.hasPasswordError ||
              this.state.hasLocalPasswordError}
            submitPasswordStatus={status}
            isPasswordSubmitEnabled={passwordButtonEnabled}
            onSubmitPassword={this.onSubmitPassword}
            password1={this.state.password1}
            password2={this.state.password2}
            onPassword1Change={this.onPassword1Change}
            onPassword2Change={this.onPassword2Change}/>
        </Dali.VBoxLayout>);
    })();
    const commentBoxButtonStyle = (() => {
      if(this.props.readonly) {
        return ProfilePage.STYLE.hidden;
      } else  {
        return null;
      }
    })();
    const profileSubmitStatus = (() => {
      if(this.state.isProfileChanged ||
          this.props.identity === this.state.newIdentity) {
        return null;
      } else {
        return this.props.submitStatus;
      }
    })();
    return (
      <PageWrapper>
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
                      value={this.state.newIdentity.firstName}
                      displaySize={this.props.displaySize}
                      onInput={this.onFirstNameChange}
                      readonly={this.props.readonly}/>
                  </FormEntry>
                  <Dali.Padding size={ProfilePage.LINE_PADDING}/>
                  <HLine color={ProfilePage.LINE_COLOR}/>
                  <Dali.Padding size={ProfilePage.LINE_PADDING}/>
                  <FormEntry name='Last Name'
                      displaySize={this.props.displaySize}>
                    <TextField
                      value={this.state.newIdentity.lastName}
                      displaySize={this.props.displaySize}
                      onInput={this.onLastNameChange}
                      readonly={this.props.readonly}/>
                  </FormEntry>
                  <Dali.Padding size={ProfilePage.LINE_PADDING}/>
                  <HLine color={ProfilePage.LINE_COLOR}/>
                  <Dali.Padding size={ProfilePage.LINE_PADDING}/>
                  <FormEntry name='Username'
                      displaySize={this.props.displaySize}>
                    <TextField
                      value={this.props.account.name.toString()}
                      displaySize={this.props.displaySize}
                      readonly/>
                  </FormEntry>
                  <Dali.Padding size={ProfilePage.LINE_PADDING}/>
                  <HLine color={ProfilePage.LINE_COLOR}/>
                  <Dali.Padding size={ProfilePage.LINE_PADDING}/>
                  <FormEntry name='Role(s)'
                      displaySize={this.props.displaySize}>
                    <div style={ProfilePage.STYLE.rolesWrapper}>
                      <RolesField roles={this.state.newRoles}
                        readonly={this.props.readonly}
                        onClick={this.onRolesChange}/>
                    </div>
                  </FormEntry>
                  <Dali.Padding size={ProfilePage.LINE_PADDING}/>
                  <HLine color={ProfilePage.LINE_COLOR}/>
                  <Dali.Padding size={ProfilePage.LINE_PADDING}/>
                  <FormEntry name='Group(s)' 
                      displaySize={this.props.displaySize}>
                    <Dali.VBoxLayout>
                      {groupsList}
                    </Dali.VBoxLayout>
                  </FormEntry>
                  <Dali.Padding size='9px'/>
                  <HLine color={ProfilePage.LINE_COLOR}/>
                  <Dali.Padding size={ProfilePage.LINE_PADDING}/>
                  <FormEntry name='Registration Date'
                      displaySize={this.props.displaySize}>
                    <TextField displaySize={this.props.displaySize}
                      value={this.props.identity.registrationTime.toString()}
                      readonly/>
                  </FormEntry>
                  <Dali.Padding size={ProfilePage.LINE_PADDING}/>
                  <HLine color={ProfilePage.LINE_COLOR}/>
                  <Dali.Padding size={ProfilePage.LINE_PADDING}/>
                  <FormEntry name='ID Number'
                      displaySize={this.props.displaySize}>
                    <TextField
                      value={this.props.account.id.toString()}
                      displaySize={this.props.displaySize}
                      readonly/>
                  </FormEntry>
                  <Dali.Padding size={ProfilePage.LINE_PADDING}/>
                  <HLine color={ProfilePage.LINE_COLOR}/>
                  <Dali.Padding size={ProfilePage.LINE_PADDING}/>
                  <FormEntry name='Email' displaySize={this.props.displaySize}>
                    <TextField
                      value={this.state.newIdentity.emailAddress}
                      displaySize={this.props.displaySize}
                      onInput={this.onEmailChange}
                      readonly={this.props.readonly}/>
                  </FormEntry>
                  <Dali.Padding size={ProfilePage.LINE_PADDING}/>
                  <HLine color={ProfilePage.LINE_COLOR}/>
                  <Dali.Padding size={ProfilePage.LINE_PADDING}/>
                  <FormEntry name='Address'
                      displaySize={this.props.displaySize}>
                    <TextField
                      value={this.state.newIdentity.addressLineOne}
                      displaySize={this.props.displaySize}
                      onInput={this.onAddressChange}
                      readonly={this.props.readonly}/>
                  </FormEntry>
                  <Dali.Padding size={ProfilePage.LINE_PADDING}/>
                  <HLine color={ProfilePage.LINE_COLOR}/>
                  <Dali.Padding size={ProfilePage.LINE_PADDING}/>
                  <FormEntry name='City' displaySize={this.props.displaySize}>
                    <TextField
                      value={this.state.newIdentity.city}
                      displaySize={this.props.displaySize}
                      onInput={this.onCityChange}
                      readonly={this.props.readonly}/>
                  </FormEntry>
                  <Dali.Padding size={ProfilePage.LINE_PADDING}/>
                  <HLine color={ProfilePage.LINE_COLOR}/>
                  <Dali.Padding size={ProfilePage.LINE_PADDING}/>
                  <FormEntry name='Province/State'
                      displaySize={this.props.displaySize}>
                    <TextField
                      value={this.state.newIdentity.province}
                      displaySize={this.props.displaySize}
                      onInput={this.onProvinceChange}
                      readonly={this.props.readonly}/>
                  </FormEntry>
                  <Dali.Padding size={ProfilePage.LINE_PADDING}/>
                  <HLine color={ProfilePage.LINE_COLOR}/>
                  <Dali.Padding size={ProfilePage.LINE_PADDING}/>
                  <FormEntry name='Country'
                      displaySize={this.props.displaySize}>
                    {countryBox}
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
                      {profileSubmitStatus}
                    <div style={ProfilePage.STYLE.buttonPadding}/>
                  </div>
                  <SubmitButton label='Save Changes'
                    displaySize={this.props.displaySize}
                    isSubmitEnabled=
                      {this.state.isProfileChanged || this.props.hasError}
                    onClick={this.onSubmitProfile}/>
                  <div style={statusMessageFooter}>
                    <div style={ProfilePage.STYLE.smallPadding}/>
                    {profileSubmitStatus}
                  </div>
                </div>
              </Dali.VBoxLayout>
              {changePasswordBox}
              <Dali.Padding size={ProfilePage.BOTTOM_PADDING}/>
            </Dali.VBoxLayout>
          </div>
          <div style={ProfilePage.STYLE.pagePadding}/>
        </div>
      </PageWrapper>);
  }

  private onFirstNameChange(value: string) {
    this.state.newIdentity.firstName = value;
    this.setState({
      newIdentity: this.state.newIdentity,
      isProfileChanged: true
    });
  }

  private onLastNameChange(value: string) {
    this.state.newIdentity.lastName = value;
    this.setState({
      newIdentity: this.state.newIdentity,
      isProfileChanged: true
    });
  }

  private onEmailChange(value: string) {
    this.state.newIdentity.emailAddress = value;
    this.setState({
      newIdentity: this.state.newIdentity,
      isProfileChanged: true
    });
  }

  private onAddressChange(value: string) {
    this.state.newIdentity.addressLineOne = value;
    this.setState({
      newIdentity: this.state.newIdentity,
      isProfileChanged: true
    });
  }

  private onCityChange(value: string) {
    this.state.newIdentity.city = value;
    this.setState({
      newIdentity: this.state.newIdentity,
      isProfileChanged: true
    });
  }
  

  private onProvinceChange(value: string) {
    this.state.newIdentity.province = value;
    this.setState({
      newIdentity: this.state.newIdentity,
      isProfileChanged: true
    });
  }

  private onCountryChange(value: Nexus.CountryCode) {
    this.state.newIdentity.country = value;
    this.setState({
      newIdentity: this.state.newIdentity,
      isProfileChanged: true
    });
  }

  private onRolesChange(role: Nexus.AccountRoles.Role) {
    if(!this.props.readonly) {
      if(this.state.newRoles.test(role)) {
        this.state.newRoles.unset(role);
      } else {
        this.state.newRoles.set(role);
      }
    }
    this.setState({
      newRoles: this.state.newRoles,
      isProfileChanged: true
    });
  }

  private onCommentChange(newComment: string) {
    this.state.newIdentity.userNotes = newComment;
    this.setState({
      newIdentity: this.state.newIdentity,
      isProfileChanged: true
    });
  }

  private onPassword1Change(newPassword: string) {
    this.setState({
      password1: newPassword,
      isPasswordChanged: true
    });
  }

  private onPassword2Change(newPassword: string) {
    this.setState({password2: newPassword});
  }

  private onSubmitPassword() {
    if(this.state.password1 === this.state.password2) {
      this.props.onSubmitPassword(this.state.password1);
      this.setState({
        hasLocalPasswordError: false,
        localPasswordMessage: ''
      });
    } else {
      this.setState({
        hasLocalPasswordError: true,
        localPasswordMessage: 'Passwords do not match'
      });
    }
    this.setState({
      password1: '',
      password2: '',
      isPasswordChanged: false
    });
  }

  private onSubmitProfile() {
    this.props.onSubmit(this.state.newRoles, this.state.newIdentity);
    this.setState({isProfileChanged: false});
  }

  private static readonly STYLE = {
    page: {
      boxSizing: 'border-box' as 'border-box',
      height: '100%',
      width: '100%',
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      justifyContent: 'space-between' as 'space-between',
      overflowY: 'auto' as 'auto'
    },
    hidden: {
      boxSizing: 'border-box' as 'border-box',
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
      boxSizing: 'border-box' as 'border-box',
      flexGrow: 1,
      maxWidth: '424px'
    },
    contentMedium: {
      boxSizing: 'border-box' as 'border-box',
      width: '732px',
      flexGrow: 0,
      flexShrink: 0
    },
    contentLarge: {
      boxSizing: 'border-box' as 'border-box',
      width: '1000px',
      flexGrow: 0,
      flexShrink: 0
    },
    pagePadding: {
      width: '30px'
    },
    rolesWrapper: {
      boxSizing: 'border-box' as 'border-box',
      marginLeft: '11px',
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      height: '34px',
      width: '122px',
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
      flexGrow: 1,
      flexShrink: 0,
      width: '100%',
      height: '18px'
    },
    mediumPadding: {
      flexGrow: 1,
      flexShrink: 0,
      width: '100%',
      height: '30px'
    },
    inlineStatusBox: {
      boxSizing: 'border-box' as 'border-box',
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      flexWrap: 'nowrap' as 'nowrap',
      alignItems: 'center' as 'center'
    },
    stackedStatusBox: {
      boxSizing: 'border-box' as 'border-box',
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
