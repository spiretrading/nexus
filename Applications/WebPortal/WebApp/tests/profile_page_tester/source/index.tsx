import * as Nexus from 'nexus';
import * as React from 'react';
import * as ReactDOM from 'react-dom';
import * as WebPortal from 'web_portal';

 /** Determines the size to render components at. */
interface Properties {
  displaySize: WebPortal.DisplaySize;
}

interface State {
  lastNameValue: string;
  someRoles: Nexus.AccountRoles;
  isPhotoFieldReadonly: boolean;
  imageSource: string;
  imagingScaling: number;
  photoFieldDisplayMode: WebPortal.DisplayMode;
}

/**  Displays a testing application. */
class TestApp extends React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    this.state = {
      lastNameValue: 'Grey',
      someRoles: new Nexus.AccountRoles(),
      imageSource: TestApp.SOME_IMAGE,
      isPhotoFieldReadonly: false,
      imagingScaling: 0,
      photoFieldDisplayMode: WebPortal.DisplayMode.DISPLAY
    };
    this.onTextInput = this.onTextInput.bind(this);
    this.onRoleClick = this.onRoleClick.bind(this);
    this.changeImage = this.changeImage.bind(this);
    this.toggleReadOnly = this.toggleReadOnly.bind(this);
    this.updateImage = this.updateImage.bind(this);
    this.toggleDisplayMode = this.toggleDisplayMode.bind(this);
  }

  public render(): JSX.Element {
    const orientation = (() => {
      if(this.props.displaySize === WebPortal.DisplaySize.SMALL) {
        return WebPortal.FormEntry.Orientation.VERTICAL;
      } else {
        return WebPortal.FormEntry.Orientation.HORIZONTAL;
      }
    })();
    return (
      <WebPortal.HBoxLayout width='100%' height='100%'>
        <WebPortal.Padding size='18px'/>
        <WebPortal.VBoxLayout width='100%' height='100%'>
          <WebPortal.Padding size='30px'/>
          <WebPortal.FormEntry name='First Name'
              readonly
              orientation={orientation}>
            <WebPortal.TextField
              value = 'Gandalf'
              displaySize={this.props.displaySize}
              disabled/>
          </WebPortal.FormEntry>
          <WebPortal.Padding size='30px'/>
          <WebPortal.FormEntry name='Last Name'
              orientation={orientation}>
            <WebPortal.TextField
              displaySize={this.props.displaySize}
              value={this.state.lastNameValue}
              onInput={this.onTextInput}/>
          </WebPortal.FormEntry>
          <WebPortal.Padding size='30px'/>
          <WebPortal.RolesField roles={this.state.someRoles}
            onClick={this.onRoleClick}/>
          <WebPortal.Padding size='30px'/>
          <WebPortal.FormEntry name='Nickname'
              readonly
              orientation={orientation}>
            <WebPortal.TextField
              value = 'Stormcrow'
              displaySize={this.props.displaySize}
              disabled/>
          </WebPortal.FormEntry>
          <WebPortal.Padding size='30px'/>
          <WebPortal.PhotoField
            displaySize={this.props.displaySize}
            displayMode={this.state.photoFieldDisplayMode}
            imageSource = {this.state.imageSource}
            readonly={this.state.isPhotoFieldReadonly}
            onUpload={this.updateImage}
            toggleUploader={this.toggleDisplayMode}
            scaling={this.state.imagingScaling}/>
        </WebPortal.VBoxLayout>
        <WebPortal.Padding size='18px'/>
        <div style={TestApp.STYLE.testingComponents}>
          <button tabIndex={-1}
              onClick={this.toggleReadOnly}>
            TOGGLE PHOTOFIELD READONLY
          </button>
          <button tabIndex={-1}
              onClick={this.changeImage}>
            CHANGE IMAGE
          </button>
        </div>
      </WebPortal.HBoxLayout>);
  }

  private onTextInput(value: string) {
    this.setState({
      lastNameValue: value
    });
  }

  private onRoleClick(role: Nexus.AccountRoles.Role) {
    if(this.state.someRoles.test(role)) {
      this.state.someRoles.unset(role);
    } else {
      this.state.someRoles.set(role);
    }
    this.setState({someRoles: this.state.someRoles});
  }

  private toggleReadOnly() {
    this.setState({
      isPhotoFieldReadonly: !this.state.isPhotoFieldReadonly
    });
  }

  private changeImage() {
    if(this.state.imageSource) {
      this.setState({
        imageSource: null
      });
    } else {
      this.setState({
        imageSource: TestApp.SOME_IMAGE
      });
    }
  }

  private updateImage(fileLocation: string, newScaling: number) {
    this.setState({
      imageSource: fileLocation,
      imagingScaling: newScaling
    });
  }

  private toggleDisplayMode() {
    if(this.state.photoFieldDisplayMode === WebPortal.DisplayMode.DISPLAY) {
      this.setState({photoFieldDisplayMode: WebPortal.DisplayMode.UPLOADING});
    } else {
      this.setState({photoFieldDisplayMode: WebPortal.DisplayMode.DISPLAY});
    }
  }

  private static STYLE = {
    testingComponents: {
      position: 'fixed' as 'fixed',
      top: 0,
      left: 0,
      zIndex: 500
    }
  };
  private static readonly SOME_IMAGE = 'https://upload.wikimedia.org/' +
  'wikipedia/commons/thumb/2/23/Close_up_of_a_black_domestic_cat.jpg/' +
  '675px-Close_up_of_a_black_domestic_cat.jpg';
}

const ResponsivePage = WebPortal.displaySizeRenderer(TestApp);
ReactDOM.render(<ResponsivePage/>, document.getElementById('main'));
