import * as Nexus from 'nexus';
import * as React from 'react';
import * as ReactDOM from 'react-dom';
import * as WebPortal from 'web_portal';
import { PhotoField } from 'web_portal';

 /** Determines the size to render components at. */
interface Properties {
  displaySize: WebPortal.DisplaySize;
}

interface State {
  lastNameValue: string;
  someRoles: Nexus.AccountRoles;
  isPhotoFieldReadonly: boolean;
  imageSource: string;
}

/**  Displays a testing application. */
class TestApp extends React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    this.state = {
      lastNameValue: 'Grey',
      someRoles: new Nexus.AccountRoles(),
      imageSource: null,
      isPhotoFieldReadonly: false
    };
    this.onTextInput = this.onTextInput.bind(this);
    this.onRoleClick = this.onRoleClick.bind(this);
    this.changeImage = this.changeImage.bind(this);
    this.toggleReadOnly = this.toggleReadOnly.bind(this);
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
          <WebPortal.PhotoField displaySize={this.props.displaySize}
          imageSource = {this.state.imageSource}
          readonly={this.state.isPhotoFieldReadonly}
          onUpload={this.changeImage}/>
        </WebPortal.VBoxLayout>
        <WebPortal.Padding size='18px'/>
        <div style={TestApp.STYLE.testingComponents}>
          <button tabIndex={-1}
              onClick={this.toggleReadOnly}>
            TOGGLE PHOTOFIELD READONLY
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
    return true;
  }

  private static STYLE = {
    testingComponents: {
      position: 'fixed' as 'fixed',
      top: 0,
      left: 0,
      zIndex: 1
    }
  };
  private static readonly SOME_IMAGE = 'https://upload.wikimedia.org/' +
    'wikipedia/commons/thumb/f/f3/Youngkitten.JPG/1024px-Youngkitten.JPG';
}

const ResponsivePage = WebPortal.displaySizeRenderer(TestApp);
ReactDOM.render(<ResponsivePage/>, document.getElementById('main'));
