import { css, StyleSheet } from 'aphrodite';
import * as Beam from 'beam';
import * as Nexus from 'nexus';
import * as React from 'react';
import { CountrySelect, DisplaySize, Input, PageLayout } from '../../..';
import { AddressField, PhotoField, PhotoFieldDisplayMode } from '../..';
import { CreateAccountModel } from './create_account_model';
import { GroupSelectionBox } from './group_selection_box';
import { GroupSuggestionModel } from './group_suggestion_model';
import { InputValidation, VALID_INPUT } from './input_validation';
import { PropertyItem } from './property_item';
import { RolesInput } from './roles_input';

interface Properties {

  /** The size of the element to display. */
  displaySize: DisplaySize;

  /** The database of available countries. */
  countryDatabase: Nexus.CountryDatabase;

  /** The model used to create the account. */
  model: CreateAccountModel;

  /** The model that provides the group suggestions. */
  groupSuggestionModel: GroupSuggestionModel;

  /** Indicates the account was created. */
  onComplete?: () => void;
}

interface State {
  isModelLoaded: boolean;
  status: CreateAccountPage.Status;
  validation: InputValidation[];
  username: string;
  identity: Nexus.AccountIdentity;
  roles: Nexus.AccountRoles;
  groupsValue: string;
  suggestedGroups: Beam.DirectoryEntry[];
  selectedGroups: Beam.DirectoryEntry[];
  photoUploaderMode: PhotoFieldDisplayMode;
  newPhoto: string;
  newScaling: number;
}

/** The page that is shown when the user wants to create a new account. */
export class CreateAccountPage extends React.Component<Properties, State> {
  public constructor(props: Properties) {
    super(props);
    this.state = {
      isModelLoaded: false,
      status: CreateAccountPage.Status.NONE,
      validation: CreateAccountPage.makeValidation(),
      username: '',
      identity: new Nexus.AccountIdentity(),
      roles: new Nexus.AccountRoles(),
      groupsValue: '',
      suggestedGroups: [],
      selectedGroups: [],
      photoUploaderMode: PhotoFieldDisplayMode.DISPLAY,
      newPhoto: '',
      newScaling: 1
    };
  }

  public render(): JSX.Element {
    if(!this.state.isModelLoaded) {
      return <div/>;
    }
    const isSmall = this.props.displaySize === DisplaySize.SMALL;
    const areaStyle = (() => {
      switch(this.props.displaySize) {
        case DisplaySize.SMALL:
          return CreateAccountPage.STYLE.areaSmall;
        case DisplaySize.MEDIUM:
          return CreateAccountPage.STYLE.areaMedium;
        case DisplaySize.LARGE:
          return CreateAccountPage.STYLE.areaLarge;
      }
    })();
    const areaA = (
      <div style={areaStyle}>
        <PhotoField
          displaySize={this.props.displaySize}
          displayMode={this.state.photoUploaderMode}
          imageSource={this.state.identity.photoId}
          onToggleUploader={this.onPhotoFieldClick}
          onSubmit={this.onPhotoSubmit}
          onNewPhotoChange={this.onPhotoChange}
          onNewScalingChange={this.onScaleChange}
          newImageSource={this.state.newPhoto}
          newScaling={this.state.newScaling}
          scaling={1}/>
        <div style={CreateAccountPage.STYLE.filler}/>
      </div>);
    const content = (() => {
      if(isSmall) {
        return (
          <React.Fragment>
            {areaA}
            <div style={CreateAccountPage.STYLE.areaPadding}/>
            {this.renderProperties()}
          </React.Fragment>);
      }
      const gap = (() => {
        if(this.props.displaySize === DisplaySize.MEDIUM) {
          return CreateAccountPage.STYLE.mediumGap;
        }
        return CreateAccountPage.STYLE.largeGap;
      })();
      return (
        <div style={CreateAccountPage.STYLE.sectionRow}>
          {areaA}
          <div style={gap}/>
          <div style={CreateAccountPage.STYLE.listArea}>
            {this.renderProperties()}
          </div>
        </div>);
    })();
    const headerPadding = (() => {
      if(isSmall) {
        return CreateAccountPage.STYLE.headerPaddingSmall;
      }
      return CreateAccountPage.STYLE.headerPaddingLarge;
    })();
    const submit = (
      <button
          className={css(this.getButtonStyle())}
          disabled={this.isSubmitDisabled()}
          onClick={this.onSubmit}>
        Create Account
      </button>);
    const submitRow = (() => {
      if(isSmall) {
        return submit;
      }
      return (
        <div style={CreateAccountPage.STYLE.submitRow}>
          <div style={CreateAccountPage.STYLE.filler}/>
          {submit}
          <div style={CreateAccountPage.STYLE.filler}/>
        </div>);
    })();
    const status = (() => {
      if(this.state.status !== CreateAccountPage.Status.UNAVAILABLE) {
        return <div style={CreateAccountPage.STYLE.statusPlaceholder}/>;
      }
      return (
        <div style={CreateAccountPage.STYLE.statusFeedback}>
          <div style={CreateAccountPage.STYLE.statusPadding}/>
          <span style={CreateAccountPage.STYLE.statusMessage}>
            Server issue
          </span>
        </div>);
    })();
    return (
      <PageLayout>
        <main style={CreateAccountPage.STYLE.main}>
          <section style={CreateAccountPage.STYLE.section}>
            <h1 style={CreateAccountPage.STYLE.header}>Create Account</h1>
            <div style={headerPadding}/>
            {content}
          </section>
          <div style={CreateAccountPage.STYLE.submitPadding}/>
          {submitRow}
          {status}
        </main>
      </PageLayout>);
  }

  public componentDidMount(): void {
    this.props.groupSuggestionModel.load().then(() => {
      for(const country of this.props.countryDatabase) {
        this.state.identity.country = country.code;
        break;
      }
      this.setState({isModelLoaded: true, identity: this.state.identity});
    });
  }

  public componentWillUnmount(): void {
    clearTimeout(this._usernameTimer);
  }

  private renderProperties(): JSX.Element {
    const Field = CreateAccountPage.Field;
    const items = [
      this.renderInput(Field.FIRST_NAME, 'text', 'given-name',
        this.state.identity.firstName, this.onFirstNameChange),
      this.renderInput(Field.LAST_NAME, 'text', 'family-name',
        this.state.identity.lastName, this.onLastNameChange),
      this.renderInput(Field.USERNAME, 'text', 'username',
        this.state.username, this.onUsernameChange),
      this.renderItem(Field.ROLES,
        <RolesInput
          displaySize={this.props.displaySize}
          roles={this.state.roles}
          isError={this.isShowingError(Field.ROLES)}
          onClick={this.onRoleClick}/>),
      this.renderItem(Field.GROUPS,
        <GroupSelectionBox
          value={this.state.groupsValue}
          onValueChange={this.onGroupsValueChange}
          displaySize={this.props.displaySize}
          selectedGroups={this.state.selectedGroups}
          suggestions={this.state.suggestedGroups}
          onAddGroup={this.onAddGroup}
          onRemoveGroup={this.onRemoveGroup}
          isError={this.isShowingError(Field.GROUPS)}/>),
      this.renderInput(Field.EMAIL, 'email', 'email',
        this.state.identity.emailAddress, this.onEmailChange),
      this.renderItem(Field.ADDRESS,
        <AddressField
          id={CreateAccountPage.IDS[Field.ADDRESS]}
          displaySize={this.props.displaySize}
          addressLineOne={this.state.identity.addressLineOne}
          addressLineTwo={this.state.identity.addressLineTwo}
          addressLineThree={this.state.identity.addressLineThree}
          onChange={this.onAddressChange}/>),
      this.renderInput(Field.CITY, 'text', 'address-level2',
        this.state.identity.city, this.onCityChange),
      this.renderInput(Field.PROVINCE, 'text', 'address-level1',
        this.state.identity.province, this.onProvinceChange),
      this.renderItem(Field.COUNTRY,
        <CountrySelect
          countryDatabase={this.props.countryDatabase}
          value={this.state.identity.country}
          style={CreateAccountPage.STYLE.countryField}
          onChange={this.onCountryChange}/>, true)
    ];
    const children = items.map((item, index) => {
      const style = (() => {
        if(index === 0) {
          return CreateAccountPage.STYLE.firstListItem;
        }
        return CreateAccountPage.STYLE.listItem;
      })();
      return (
        <li key={CreateAccountPage.LABELS[index]} style={style}>{item}</li>);
    });
    return (
      <ul style={CreateAccountPage.STYLE.list}>{children}</ul>);
  }

  private renderInput(field: CreateAccountPage.Field, type: string,
      autoComplete: string, value: string,
      onChange: (value: string) => void): JSX.Element {
    return this.renderItem(field,
      <Input
        id={CreateAccountPage.IDS[field]}
        name={CreateAccountPage.IDS[field]}
        type={type}
        autoComplete={autoComplete}
        value={value}
        style={this.getInputStyle(field)}
        onChange={(event: React.ChangeEvent<HTMLInputElement>) =>
          onChange(event.target.value)}/>);
  }

  private renderItem(field: CreateAccountPage.Field, item: JSX.Element,
      isLast?: boolean): JSX.Element {
    return (
      <PropertyItem
          displaySize={this.props.displaySize}
          label={CreateAccountPage.LABELS[field]}
          htmlFor={CreateAccountPage.IDS[field]}
          validation={this.state.validation[field]}
          isLast={isLast}>
        {item}
      </PropertyItem>);
  }

  private getFontSize(): string {
    if(this.props.displaySize === DisplaySize.SMALL) {
      return '1rem';
    }
    return '0.875rem';
  }

  private getButtonStyle() {
    if(this.props.displaySize === DisplaySize.SMALL) {
      return CreateAccountPage.DYNAMIC_STYLE.buttonSmall;
    }
    return CreateAccountPage.DYNAMIC_STYLE.buttonLarge;
  }

  private getInputStyle(field: CreateAccountPage.Field): React.CSSProperties {
    const style = {
      ...CreateAccountPage.STYLE.inputField, fontSize: this.getFontSize()
    };
    if(this.isShowingError(field)) {
      return {...style, borderColor: '#E63F44'};
    }
    return style;
  }

  private isShowingError(field: CreateAccountPage.Field): boolean {
    const validation = this.state.validation[field];
    return !validation.valid && validation.showError;
  }

  private isSubmitDisabled(): boolean {
    return this.state.status === CreateAccountPage.Status.IN_PROGRESS ||
      !this.state.validation.every(input => input.valid);
  }

  private markValid(field: CreateAccountPage.Field): void {
    this.updateValidation(field, {
      valid: true,
      error: CreateAccountModel.ValidationError.NONE,
      showError: true
    });
  }

  private updateValidation(field: CreateAccountPage.Field,
      validation: InputValidation): void {
    const inputs = this.state.validation.slice();
    inputs[field] = validation;
    this.setState({validation: inputs});
  }

  private onPhotoFieldClick = () => {
    if(this.state.photoUploaderMode === PhotoFieldDisplayMode.DISPLAY) {
      this.setState({
        photoUploaderMode: PhotoFieldDisplayMode.UPLOADING,
        newPhoto: this.state.identity.photoId,
        newScaling: 1
      });
    } else {
      this.setState({
        photoUploaderMode: PhotoFieldDisplayMode.DISPLAY,
        newPhoto: this.state.identity.photoId
      });
    }
  }

  private onPhotoChange = (photo: string) => {
    this.setState({newPhoto: photo});
  }

  private onScaleChange = (scaling: number) => {
    this.setState({newScaling: scaling});
  }

  private onPhotoSubmit = (newFileLocation: string, scaling: number) => {
    this.state.identity.photoId = newFileLocation;
    this.setState({identity: this.state.identity});
  }

  private onRoleClick = (role: Nexus.AccountRoles.Role) => {
    if(this.state.roles.test(role)) {
      this.state.roles.unset(role);
    } else {
      this.state.roles.set(role);
    }
    this.setState({roles: this.state.roles});
    this.markValid(CreateAccountPage.Field.ROLES);
  }

  private onFirstNameChange = (value: string) => {
    this.state.identity.firstName = value;
    this.setState({identity: this.state.identity});
    this.markValid(CreateAccountPage.Field.FIRST_NAME);
  }

  private onLastNameChange = (value: string) => {
    this.state.identity.lastName = value;
    this.setState({identity: this.state.identity});
    this.markValid(CreateAccountPage.Field.LAST_NAME);
  }

  private onEmailChange = (value: string) => {
    this.state.identity.emailAddress = value;
    this.setState({identity: this.state.identity});
    this.markValid(CreateAccountPage.Field.EMAIL);
  }

  private onAddressChange = (addressLineOne: string,
      addressLineTwo: string, addressLineThree: string) => {
    this.state.identity.addressLineOne = addressLineOne;
    this.state.identity.addressLineTwo = addressLineTwo;
    this.state.identity.addressLineThree = addressLineThree;
    this.setState({identity: this.state.identity});
  }

  private onCityChange = (value: string) => {
    this.state.identity.city = value;
    this.setState({identity: this.state.identity});
  }

  private onProvinceChange = (value: string) => {
    this.state.identity.province = value;
    this.setState({identity: this.state.identity});
  }

  private onCountryChange = (value: Nexus.CountryCode) => {
    this.state.identity.country = value;
    this.setState({identity: this.state.identity});
  }

  private onUsernameChange = (value: string) => {
    this.setState({username: value});
    this.markValid(CreateAccountPage.Field.USERNAME);
    clearTimeout(this._usernameTimer);
    this._usernameTimer = setTimeout(() => {
      this.validateUsername(value);
    }, CreateAccountPage.VALIDATION_DELAY);
  }

  private async validateUsername(username: string): Promise<void> {
    const request = ++this._usernameRequest;
    const error = await this.props.model.validateUsername(username);
    if(request !== this._usernameRequest ||
        this.state.username !== username) {
      return;
    }
    this.updateValidation(CreateAccountPage.Field.USERNAME, {
      valid: error === CreateAccountModel.ValidationError.NONE,
      error: error,
      showError: true
    });
  }

  private onGroupsValueChange = async (value: string) => {
    this.setState({groupsValue: value});
    const suggestions =
      await this.props.groupSuggestionModel.loadSuggestions(value);
    this.setState({
      suggestedGroups: suggestions.filter(
        group => this.state.selectedGroups.indexOf(group) < 0)
    });
  }

  private onAddGroup = (group: Beam.DirectoryEntry) => {
    if(this.state.selectedGroups.indexOf(group) >= 0 ||
        this.state.selectedGroups.length >=
        CreateAccountPage.MAX_NUMBER_OF_GROUPS) {
      return;
    }
    this.state.selectedGroups.push(group);
    this.setState({
      selectedGroups: this.state.selectedGroups,
      groupsValue: '',
      suggestedGroups: []
    });
    this.markValid(CreateAccountPage.Field.GROUPS);
  }

  private onRemoveGroup = (group: Beam.DirectoryEntry) => {
    this.state.selectedGroups.splice(
      this.state.selectedGroups.indexOf(group), 1);
    this.setState({selectedGroups: this.state.selectedGroups});
  }

  private onSubmit = async () => {
    const Field = CreateAccountPage.Field;
    const ValidationError = CreateAccountModel.ValidationError;
    const inputs = this.state.validation.slice();
    const markRequired = (
        field: CreateAccountPage.Field, isEmpty: boolean) => {
      inputs[field] = (() => {
        if(isEmpty) {
          return {
            valid: false, error: ValidationError.REQUIRED, showError: true
          };
        }
        return {valid: true, error: ValidationError.NONE, showError: true};
      })();
    };
    markRequired(Field.FIRST_NAME, this.state.identity.firstName === '');
    markRequired(Field.LAST_NAME, this.state.identity.lastName === '');
    markRequired(Field.EMAIL, this.state.identity.emailAddress === '');
    markRequired(Field.GROUPS, this.state.selectedGroups.length === 0);
    markRequired(Field.ROLES, !this.hasRole());
    const username = CreateAccountModel.normalizeUsername(this.state.username);
    const usernameError = await this.props.model.validateUsername(username);
    inputs[Field.USERNAME] = {
      valid: usernameError === ValidationError.NONE,
      error: usernameError,
      showError: true
    };
    this.setState({validation: inputs});
    if(!inputs.every(input => input.valid)) {
      return;
    }
    this.setState({status: CreateAccountPage.Status.IN_PROGRESS});
    try {
      await this.props.model.createAccount(username,
        this.state.selectedGroups[0], this.state.identity, this.state.roles);
      this.setState({status: CreateAccountPage.Status.COMPLETE});
      this.props.onComplete?.();
    } catch(error: any) {
      if(error?.code === CreateAccountPage.CONFLICT) {
        inputs[Field.USERNAME] = {
          valid: false, error: ValidationError.DUPLICATE, showError: true
        };
        this.setState({
          status: CreateAccountPage.Status.NONE, validation: inputs
        });
      } else {
        this.setState({status: CreateAccountPage.Status.UNAVAILABLE});
      }
    }
  }

  private hasRole(): boolean {
    return this.state.roles.test(Nexus.AccountRoles.Role.ADMINISTRATOR) ||
      this.state.roles.test(Nexus.AccountRoles.Role.MANAGER) ||
      this.state.roles.test(Nexus.AccountRoles.Role.TRADER) ||
      this.state.roles.test(Nexus.AccountRoles.Role.SERVICE);
  }

  private static makeValidation(): InputValidation[] {
    const validation = [] as InputValidation[];
    for(const label of CreateAccountPage.LABELS) {
      validation.push({...VALID_INPUT});
    }
    validation[CreateAccountPage.Field.USERNAME] = {
      valid: false,
      error: CreateAccountModel.ValidationError.REQUIRED,
      showError: false
    };
    return validation;
  }

  private static readonly LABELS = ['First Name', 'Last Name', 'Username',
    'Role(s)', 'Group(s)', 'Email', 'Address', 'City', 'Province/State',
    'Country'];
  private static readonly IDS = ['first-name', 'last-name', 'username', 'role',
    'group', 'email', 'address', 'city', 'province-state', 'country'];
  private static readonly STYLE: Record<string, React.CSSProperties> = {
    main: {
      boxSizing: 'border-box',
      flexGrow: 1,
      display: 'flex',
      flexDirection: 'column',
      padding: '18px 18px 40px',
      fontFamily: '"Roboto", system-ui, sans-serif',
      fontWeight: 400,
      color: '#333333'
    },
    section: {
      display: 'flex',
      flexDirection: 'column',
      padding: '0 0 30px',
      borderBottom: '1px solid #E6E6E6'
    },
    header: {
      margin: 0,
      fontSize: '1.125rem',
      fontWeight: 'inherit'
    },
    headerPaddingSmall: {
      height: '30px',
      flexShrink: 0
    },
    headerPaddingLarge: {
      height: '60px',
      flexShrink: 0
    },
    areaSmall: {
      display: 'flex',
      flexDirection: 'column'
    },
    areaMedium: {
      width: '280px',
      minWidth: '280px',
      display: 'flex',
      flexDirection: 'column'
    },
    areaLarge: {
      width: '380px',
      minWidth: '380px',
      display: 'flex',
      flexDirection: 'column'
    },
    areaPadding: {
      height: '30px',
      flexShrink: 0
    },
    sectionRow: {
      display: 'flex',
      flexDirection: 'row'
    },
    mediumGap: {
      width: '30px',
      minWidth: '30px',
      flexShrink: 0
    },
    largeGap: {
      width: '100px',
      minWidth: '100px',
      flexShrink: 0
    },
    listArea: {
      flexGrow: 1,
      minWidth: 0
    },
    list: {
      margin: 0,
      padding: 0,
      listStyle: 'none'
    },
    firstListItem: {
      marginTop: 0
    },
    listItem: {
      marginTop: '20px'
    },
    filler: {
      flexGrow: 1
    },
    inputField: {
      width: '100%',
      boxSizing: 'border-box'
    },
    countryField: {
      width: '100%'
    },
    submitPadding: {
      height: '30px',
      flexShrink: 0
    },
    submitRow: {
      display: 'flex',
      flexDirection: 'row'
    },
    statusPlaceholder: {
      height: '34px',
      flexShrink: 0
    },
    statusFeedback: {
      display: 'flex',
      flexDirection: 'column'
    },
    statusPadding: {
      height: '18px',
      flexShrink: 0
    },
    statusMessage: {
      textAlign: 'center',
      color: '#E63F44'
    }
  };
  private static readonly DYNAMIC_STYLE = StyleSheet.create({
    buttonSmall: {
      boxSizing: 'border-box',
      width: '100%',
      height: '34px',
      backgroundColor: '#684BC7',
      fontFamily: 'inherit',
      fontSize: 'inherit',
      fontWeight: 'inherit',
      color: '#FFFFFF',
      border: 'none',
      outline: 0,
      borderRadius: 1,
      cursor: 'pointer',
      ':active': {
        backgroundColor: '#4B23A0'
      },
      ':focus': {
        backgroundColor: '#4B23A0'
      },
      ':hover': {
        backgroundColor: '#4B23A0'
      },
      ':disabled': {
        backgroundColor: '#F8F8F8',
        color: '#8C8C8C',
        cursor: 'default'
      }
    },
    buttonLarge: {
      boxSizing: 'border-box',
      width: '246px',
      minWidth: '246px',
      height: '34px',
      backgroundColor: '#684BC7',
      fontFamily: 'inherit',
      fontSize: 'inherit',
      fontWeight: 'inherit',
      color: '#FFFFFF',
      border: 'none',
      outline: 0,
      borderRadius: 1,
      cursor: 'pointer',
      ':active': {
        backgroundColor: '#4B23A0'
      },
      ':focus': {
        backgroundColor: '#4B23A0'
      },
      ':hover': {
        backgroundColor: '#4B23A0'
      },
      ':disabled': {
        backgroundColor: '#F8F8F8',
        color: '#8C8C8C',
        cursor: 'default'
      }
    }
  });
  private static readonly CONFLICT = 409;
  private static readonly MAX_NUMBER_OF_GROUPS = 1;
  private static readonly VALIDATION_DELAY = 300;
  private _usernameTimer: ReturnType<typeof setTimeout>;
  private _usernameRequest = 0;
}

export namespace CreateAccountPage {

  /** Lists the account creation states. */
  export enum Status {

    /** Default state. */
    NONE,

    /** The account is being created. */
    IN_PROGRESS,

    /** The server is unavailable. */
    UNAVAILABLE,

    /** The account creation succeeded. */
    COMPLETE
  }

  /** Lists the inputs shown on the page. */
  export enum Field {
    FIRST_NAME,
    LAST_NAME,
    USERNAME,
    ROLES,
    GROUPS,
    EMAIL,
    ADDRESS,
    CITY,
    PROVINCE,
    COUNTRY
  }
}
